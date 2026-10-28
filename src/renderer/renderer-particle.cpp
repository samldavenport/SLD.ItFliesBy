#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "ifb-particle.hpp"
#include "ifb-quad.hpp"
#include "render-buffers.cpp"
#include "ifb-renderer.hpp"
#include "renderer-internal.hpp"
#include "renderer-camera.cpp"
#include "renderer-grid.cpp"
#include "renderer-quad.cpp"
#include "renderer-viewport.cpp"
#include "sld-math-mat4.hpp"
#include "sld-opengl.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // an offscreen render of every active particle emitter, with its own camera
    // the particles are quads, so they draw with the quad shader and elements
    struct renderer_particle_view {
        renderer_camera*   camera;
        renderer_viewport* viewport;
        render_vertex_buffer vertices;
        bool               is_requested;
        u32                requested_width;
        u32                requested_height;
        u32                width;
        u32                height;
        color_rgba_f32     clear_color;
        struct {
            gl_framebuffer  framebuffer;
            gl_texture      texture;
            gl_renderbuffer depth_stencil;
            gl_vertex       vertex;
            gl_buffer       buf_vertex;
        } gl;
    };

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // the texture and depth buffer are the size of the view
    inline void
    renderer_particle_view_resize(
        renderer_particle_view* view,
        gl_context*             gl,
        const u32               width,
        const u32               height) {

        bool gl_ok = true;
        gl_ok &= gl_texture_set_storage_rgba8              (gl, view->gl.texture,       width, height);
        gl_ok &= gl_renderbuffer_set_storage_depth_stencil (gl, view->gl.depth_stencil, width, height);
        assert(gl_ok);

        view->width            = width;
        view->height           = height;
        view->viewport->width  = (f32)width;
        view->viewport->height = (f32)height;
    }

    IFB_INTERNAL renderer_particle_view*
    renderer_particle_view_create(
        void) {

        auto view = (renderer_particle_view*)renderer_memory_alloc(sizeof(renderer_particle_view));
        assert(view);

        view->camera   = renderer_camera_create();
        view->viewport = renderer_viewport_create();
        assert(view->camera);
        assert(view->viewport);

        return(view);
    }

    IFB_INTERNAL void
    renderer_particle_view_init(
        renderer_particle_view* view,
        gl_context*             gl,
        render_buffers*         buffers,
        renderer_quad_shader*   shdr_quad) {

        static const auto& cfg = config_instance();

        assert(view);
        assert(gl);
        assert(buffers);
        assert(shdr_quad);

        // the camera looks at the emitters from the front
        renderer_viewport_init     (view->viewport);
        renderer_camera_init       (view->camera);
        renderer_camera_set_origin (view->camera, { 0.0f, 0.5f, 2.0f });
        renderer_camera_set_target (view->camera, { 0.0f, 0.5f, 0.0f });

        view->is_requested     = false;
        view->requested_width  = 0;
        view->requested_height = 0;
        view->clear_color.r    = 0.08f;
        view->clear_color.g    = 0.08f;
        view->clear_color.b    = 0.10f;
        view->clear_color.a    = 1.00f;

        // the particles share the quad elements, so we can render as many particles as quads
        view->vertices.capacity = cfg.quad_capacity * sizeof(quad_vertices);
        view->vertices.size     = 0;
        view->vertices.stride   = sizeof(quad_vertex);
        view->vertices.data     = (byte*)renderer_memory_alloc(view->vertices.capacity);
        assert(view->vertices.data != NULL);

        // create gl objects
        view->gl.framebuffer   = gl_framebuffer_create  (gl);
        view->gl.texture       = gl_texture_create      (gl);
        view->gl.depth_stencil = gl_renderbuffer_create (gl);
        view->gl.vertex        = gl_vertex_create       (gl);
        view->gl.buf_vertex    = gl_buffer_create       (gl);

        // the view is resized to fit whatever requests it
        renderer_particle_view_resize(view, gl, cfg.window_start_width, cfg.window_start_height);

        bool gl_ok = true;
        gl_ok &= gl_framebuffer_attach_color_texture (gl, view->gl.framebuffer, view->gl.texture);
        gl_ok &= gl_framebuffer_attach_depth_stencil (gl, view->gl.framebuffer, view->gl.depth_stencil);
        gl_ok &= gl_framebuffer_is_complete          (gl, view->gl.framebuffer);
        assert(gl_ok);

        // define vertex, the same layout and elements as the quads
        const u32 vertex_size = view->vertices.stride;
        gl_ok &= gl_context_set_vertex_object  (gl, view->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, view->gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (gl, shdr_quad->gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (gl, view->gl.buf_vertex, view->vertices.data, view->vertices.capacity);
        gl_ok &= gl_vertex_add_f32x3           (gl, view->gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_f32x4           (gl, view->gl.vertex, vertex_size, 1, sizeof(vec3));
        gl_ok &= gl_vertex_add_f32x2           (gl, view->gl.vertex, vertex_size, 2, sizeof(vec3) + sizeof(color_rgba_f32));
        gl_ok &= gl_vertex_add_f32x1           (gl, view->gl.vertex, vertex_size, 3, sizeof(vec3) + sizeof(color_rgba_f32) + sizeof(vec2));
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_particle_view_request(
        renderer_particle_view* view,
        const u32               width,
        const u32               height) {

        assert(view);
        if (width == 0 || height == 0) return;

        view->is_requested     = true;
        view->requested_width  = width;
        view->requested_height = height;
    }

    IFB_INTERNAL void
    renderer_particle_view_draw(
        renderer_particle_view* view,
        gl_context*             gl,
        render_buffers*         buffers,
        renderer_quad_shader*   shdr_quad,
        renderer_grid_shader*   shdr_grid) {

        static const mat4 m = mat4_identity();

        assert(view);
        assert(gl);
        assert(buffers);
        assert(shdr_quad);
        assert(shdr_grid);

        // the view only draws on the frames it is requested
        if (!view->is_requested) return;
        view->is_requested = false;

        if (view->width != view->requested_width || view->height != view->requested_height) {
            renderer_particle_view_resize(view, gl, view->requested_width, view->requested_height);
        }

        // calculate view projection
        mat4 proj;
        mat4 cam;
        renderer_viewport_get_xform (view->viewport, proj);
        renderer_camera_get_xform   (view->camera,   cam);
        const mat4 view_proj_xform = mat4_multiply(proj, cam);

        // particles -> buffer
        orientation cam_ori;
        renderer_camera_get_orientation(view->camera, cam_ori);
        view->vertices.size = particle_mngr_render_buffer(
            view->vertices.data,
            view->vertices.capacity,
            cam_ori
        );

        // draw to the texture, the window clear color is put back after
        color_rgba_f32 window_clear_color = gl->clear_color;
        gl_context_set_framebuffer     (gl, view->gl.framebuffer);
        gl_context_update_viewport     (gl, 0, 0, view->width, view->height);
        gl_context_set_clear_color_hex (gl, view->clear_color);
        gl_context_clear_viewport      (gl);
        gl_context_set_clear_color_hex (gl, window_clear_color);

        renderer_grid_draw(shdr_grid, gl, buffers, view_proj_xform);

        // buffer -> shader
        const u32 element_count = (view->vertices.count() / QUAD_VERTEX_COUNT) * QUAD_ELEMENT_COUNT;
        if (element_count != 0) {

            gl_context_set_shader_program (gl, shdr_quad->gl.program);
            gl_context_set_vertex_object  (gl, view->gl.vertex);
            gl_context_set_buffer_vertex  (gl, view->gl.buf_vertex);
            gl_context_set_buffer_element (gl, shdr_quad->gl.buf_element);
            gl_buffer_update_vertex_data  (gl, view->gl.buf_vertex, view->vertices.data, view->vertices.size);
            gl_uniform_set_mat4           (gl, shdr_quad->gl.unif_mat4_model,     m.m);
            gl_uniform_set_mat4           (gl, shdr_quad->gl.unif_mat4_view_proj, view_proj_xform.m);
            gl_context_draw_elements      (gl, element_count);
        }

        // draw to the window again
        gl_context_set_framebuffer_default(gl);
    }
};
