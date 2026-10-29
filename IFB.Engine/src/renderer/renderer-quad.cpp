#pragma once

#include "ifb.hpp"
#include "render-buffers.cpp"
#include "eng-renderer.hpp"
#include "sld-math-mat4.hpp"
#include "sld-opengl.hpp"
#include <cassert>
#include "eng-quad.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr char QUAD_UNIFORM_NAME_MAT4_VIEW_PROJ [] = "u_mat4_view_proj";
    static constexpr char QUAD_UNIFORM_NAME_MAT4_MODEL     [] = "u_mat4_model";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct renderer_quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_buffer  buf_element;
            gl_uniform unif_mat4_view_proj;
            gl_uniform unif_mat4_model;
        } gl;
    };

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL renderer_quad_shader*
    renderer_quad_shader_create(
        void) {

        auto shdr = (renderer_quad_shader*)renderer_memory_alloc(sizeof(renderer_quad_shader));
        assert(shdr);

        return(shdr);

    }

    IFB_INTERNAL void
    renderer_quad_shader_init(
        renderer_quad_shader*         shdr,
        gl_context*                   gl,
        render_buffers*               buffers,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(shdr);
        assert(gl);
        assert(buffers);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);
        assert(sizeof(vec2)           == 8);

        // the buffers need to be allocated before the shader is initialized
        auto* quad_buf_vtx   = buffers->quad_vertices;
        auto* quad_buf_elmnt = buffers->quad_elements;
        assert(quad_buf_vtx);
        assert(quad_buf_vtx->data       != NULL);
        assert(quad_buf_vtx->capacity   != 0);
        assert(quad_buf_vtx->stride     == sizeof(vec3) + sizeof(color_rgba_f32) + sizeof(vec2) + sizeof(f32));
        assert(quad_buf_elmnt);
        assert(quad_buf_elmnt->data     != NULL);
        assert(quad_buf_elmnt->capacity != 0);

        // create gl objects
        shdr->gl.program          = gl_shader_program_create        (gl);
        shdr->gl.vertex           = gl_vertex_create                (gl);
        shdr->gl.buf_vertex       = gl_buffer_create                (gl);
        shdr->gl.buf_element      = gl_buffer_create                (gl);
        const gl_shader shdr_vtx  = gl_shader_stage_create_vertex   (gl);
        const gl_shader shdr_frg  = gl_shader_stage_create_fragment (gl);

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (gl, shdr->gl.program);
        gl_shader_stage_destroy                      (gl, shdr_vtx);
        gl_shader_stage_destroy                      (gl, shdr_frg);
        assert(gl_ok);

        // get uniform locations
        shdr->gl.unif_mat4_view_proj   = gl_uniform_get_location(gl, shdr->gl.program, QUAD_UNIFORM_NAME_MAT4_VIEW_PROJ);
        shdr->gl.unif_mat4_model       = gl_uniform_get_location(gl, shdr->gl.program, QUAD_UNIFORM_NAME_MAT4_MODEL);
        gl_ok &= (
            shdr->gl.unif_mat4_view_proj   != GL_UNIFORM_INVALID &&
            shdr->gl.unif_mat4_model       != GL_UNIFORM_INVALID
        );
        assert(gl_ok);

        // define vertex
        // the gl buffers are sized to hold everything the render buffers can,
        // the elements never change so they only need to be sent once
        const u32 vertex_size = quad_buf_vtx->stride;
        gl_ok &= gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (gl, shdr->gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr->gl.buf_vertex,  quad_buf_vtx->data,          quad_buf_vtx->capacity);
        gl_ok &= gl_buffer_set_element_data    (gl, shdr->gl.buf_element, (byte*)quad_buf_elmnt->data, quad_buf_elmnt->capacity);
        gl_ok &= gl_vertex_add_f32x3           (gl, shdr->gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_f32x4           (gl, shdr->gl.vertex, vertex_size, 1, sizeof(vec3));
        gl_ok &= gl_vertex_add_f32x2           (gl, shdr->gl.vertex, vertex_size, 2, sizeof(vec3) + sizeof(color_rgba_f32));
        gl_ok &= gl_vertex_add_f32x1           (gl, shdr->gl.vertex, vertex_size, 3, sizeof(vec3) + sizeof(color_rgba_f32) + sizeof(vec2));
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_quad_draw(
        renderer_quad_shader* shdr,
        gl_context*           gl,
        render_buffers*       buffers,
        const mat4&           view_proj_xform) {

        static const mat4 m = mat4_identity();

        assert(shdr);
        assert(gl);
        assert(buffers);

        // if nothing was rendered to the buffers, we're done
        auto*     quad_buf_vtx   = buffers->quad_vertices;
        auto*     quad_buf_elmnt = buffers->quad_elements;
        const u32 element_count  = quad_buf_elmnt->count();
        if (quad_buf_vtx->size == 0 || element_count == 0) {
            return;
        }

        // set the texture
        renderer_use_sprite_atlas_texture();

        // draw elements
        gl_context_set_shader_program (gl, shdr->gl.program);
        gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_context_set_buffer_element (gl, shdr->gl.buf_element);
        gl_buffer_update_vertex_data  (gl, shdr->gl.buf_vertex, quad_buf_vtx->data, quad_buf_vtx->size);
        gl_uniform_set_mat4           (gl, shdr->gl.unif_mat4_model,     m.m);
        gl_uniform_set_mat4           (gl, shdr->gl.unif_mat4_view_proj, view_proj_xform.m);
        gl_context_draw_elements      (gl, element_count);
    }
};
