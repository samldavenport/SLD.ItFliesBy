#pragma once

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "eng-quad.hpp"
#include "render-buffers.cpp"
#include "eng-renderer.hpp"
#include "renderer-quad.cpp"
#include "sld-math-mat4.hpp"
#include "sld-opengl.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // quads the game pushes every frame, not tied to entities
    // they're drawn with the quad shader, so they share its elements and vertex layout
    //
    // world quads are in world units and draw with the camera, after the entity quads
    // screen quads are in pixels from the top left, y down, and draw last, over everything
    //
    // the depth test always passes, so in each layer the quads draw in the order they were pushed
    struct renderer_immediate {
        render_vertex_buffer world;
        render_vertex_buffer screen;
        u32                  screen_width;  // the pixels the screen layer is laid out in, 0 for the window size
        u32                  screen_height;
        struct {
            gl_vertex vertex_world;
            gl_buffer buf_world;
            gl_vertex vertex_screen;
            gl_buffer buf_screen;
        } gl;
    };

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    inline void
    renderer_immediate_init_layer(
        render_vertex_buffer* layer,
        gl_context*           gl,
        renderer_quad_shader* shdr_quad,
        gl_vertex&            vertex,
        gl_buffer&            buffer) {

        static const auto& cfg = config_instance();

        // the layers share the quad elements, so they can hold as many quads as the quad buffer
        layer->capacity = cfg.quad_capacity * sizeof(quad_vertices);
        layer->size     = 0;
        layer->stride   = sizeof(quad_vertex);
        layer->data     = (byte*)renderer_memory_alloc(layer->capacity);
        assert(layer->data != NULL);

        vertex = gl_vertex_create (gl);
        buffer = gl_buffer_create (gl);

        // the same layout as the quads
        bool gl_ok = true;
        const u32 vertex_size = layer->stride;
        gl_ok &= gl_context_set_vertex_object  (gl, vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, buffer);
        gl_ok &= gl_context_set_buffer_element (gl, shdr_quad->gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (gl, buffer, layer->data, layer->capacity);
        gl_ok &= gl_vertex_add_f32x3           (gl, vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_f32x4           (gl, vertex, vertex_size, 1, sizeof(vec3));
        gl_ok &= gl_vertex_add_f32x2           (gl, vertex, vertex_size, 2, sizeof(vec3) + sizeof(color_rgba_f32));
        gl_ok &= gl_vertex_add_f32x1           (gl, vertex, vertex_size, 3, sizeof(vec3) + sizeof(color_rgba_f32) + sizeof(vec2));
        assert(gl_ok);
        (void)gl_ok;
    }

    inline void
    renderer_immediate_draw_layer(
        render_vertex_buffer* layer,
        gl_context*           gl,
        renderer_quad_shader* shdr_quad,
        const gl_vertex       vertex,
        const gl_buffer       buffer,
        const mat4&           view_proj_xform) {

        static const mat4 m = mat4_identity();

        const u32 element_count = (layer->count() / QUAD_VERTEX_COUNT) * QUAD_ELEMENT_COUNT;
        if (element_count == 0) return;

        renderer_use_quad_textures();

        gl_context_set_shader_program (gl, shdr_quad->gl.program);
        gl_context_set_vertex_object  (gl, vertex);
        gl_context_set_buffer_vertex  (gl, buffer);
        gl_context_set_buffer_element (gl, shdr_quad->gl.buf_element);
        gl_buffer_update_vertex_data  (gl, buffer, layer->data, layer->size);
        gl_uniform_set_mat4           (gl, shdr_quad->gl.unif_mat4_model,     m.m);
        gl_uniform_set_mat4           (gl, shdr_quad->gl.unif_mat4_view_proj, view_proj_xform.m);
        gl_context_draw_elements      (gl, element_count);
    }

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL renderer_immediate*
    renderer_immediate_create(
        void) {

        auto imm = (renderer_immediate*)renderer_memory_alloc(sizeof(renderer_immediate));
        assert(imm);

        return(imm);
    }

    IFB_INTERNAL void
    renderer_immediate_init(
        renderer_immediate*   imm,
        gl_context*           gl,
        renderer_quad_shader* shdr_quad) {

        assert(imm);
        assert(gl);
        assert(shdr_quad);

        imm->screen_width  = 0;
        imm->screen_height = 0;

        renderer_immediate_init_layer(&imm->world,  gl, shdr_quad, imm->gl.vertex_world,  imm->gl.buf_world);
        renderer_immediate_init_layer(&imm->screen, gl, shdr_quad, imm->gl.vertex_screen, imm->gl.buf_screen);
    }

    IFB_INTERNAL void
    renderer_immediate_clear(
        renderer_immediate* imm) {

        assert(imm);

        imm->world.size  = 0;
        imm->screen.size = 0;
    }

    // false if the layer is full
    IFB_INTERNAL bool
    renderer_immediate_push(
        render_vertex_buffer* layer,
        const quad_vertices&  vertices) {

        assert(layer);

        if (layer->size + sizeof(quad_vertices) > layer->capacity) return(false);

        auto* dst = (quad_vertices*)&layer->data[layer->size];
        *dst = vertices;
        layer->size += sizeof(quad_vertices);
        return(true);
    }

    IFB_INTERNAL void
    renderer_immediate_draw_world(
        renderer_immediate*   imm,
        gl_context*           gl,
        renderer_quad_shader* shdr_quad,
        const mat4&           view_proj_xform) {

        assert(imm);
        assert(gl);
        assert(shdr_quad);

        renderer_immediate_draw_layer(
            &imm->world,
            gl,
            shdr_quad,
            imm->gl.vertex_world,
            imm->gl.buf_world,
            view_proj_xform
        );
    }

    // width and height are the pixels the screen layer is laid out in, they're stretched to the window
    IFB_INTERNAL void
    renderer_immediate_draw_screen(
        renderer_immediate*   imm,
        gl_context*           gl,
        renderer_quad_shader* shdr_quad,
        const f32             width,
        const f32             height) {

        assert(imm);
        assert(gl);
        assert(shdr_quad);
        assert(width  > 0.0f);
        assert(height > 0.0f);

        // orthographic, (0, 0) is the top left and (width, height) is the bottom right
        // the matrices are row major and the vectors are columns, translation is the last column
        mat4 ortho = mat4_identity();
        ortho.r0c0 =  2.0f / width;
        ortho.r0c3 = -1.0f;
        ortho.r1c1 = -2.0f / height;
        ortho.r1c3 =  1.0f;
        ortho.r2c2 = -1.0f;

        renderer_immediate_draw_layer(
            &imm->screen,
            gl,
            shdr_quad,
            imm->gl.vertex_screen,
            imm->gl.buf_screen,
            ortho
        );
    }
};
