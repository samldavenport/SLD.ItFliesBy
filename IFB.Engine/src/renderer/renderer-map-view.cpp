#pragma once

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "render-buffers.cpp"
#include "eng-renderer.hpp"
#include "renderer-camera.cpp"
#include "renderer-grid.cpp"
#include "renderer-tile.cpp"
#include "renderer-viewport.cpp"
#include "sld-math-mat4.hpp"
#include "sld-opengl.hpp"
#include "eng-map.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // an offscreen render of one map, with its own camera, for tools like the map builder
    // it draws with the same tile shader, atlas, and projection as the game,
    // so the map looks the way it does in the game
    struct renderer_map_view {
        renderer_camera*   camera;
        renderer_viewport* viewport;
        hnd_map            h_map;
        bool               is_requested;
        bool               is_grid_visible;
        u32                requested_width;
        u32                requested_height;
        u32                width;
        u32                height;
        color_rgba_f32     clear_color;
        struct {
            gl_framebuffer  framebuffer;
            gl_texture      texture;
            gl_renderbuffer depth_stencil;
        } gl;
    };

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // the texture and depth buffer are the size of the view
    inline void
    renderer_map_view_resize(
        renderer_map_view* view,
        gl_context*        gl,
        const u32          width,
        const u32          height) {

        bool gl_ok = true;
        gl_ok &= gl_texture_set_storage_rgba8              (gl, view->gl.texture,       width, height);
        gl_ok &= gl_renderbuffer_set_storage_depth_stencil (gl, view->gl.depth_stencil, width, height);
        assert(gl_ok);

        view->width            = width;
        view->height           = height;
        view->viewport->width  = (f32)width;
        view->viewport->height = (f32)height;
    }

    IFB_INTERNAL renderer_map_view*
    renderer_map_view_create(
        void) {

        auto view = (renderer_map_view*)renderer_memory_alloc(sizeof(renderer_map_view));
        assert(view);

        view->camera   = renderer_camera_create();
        view->viewport = renderer_viewport_create();
        assert(view->camera);
        assert(view->viewport);

        return(view);
    }

    IFB_INTERNAL void
    renderer_map_view_init(
        renderer_map_view* view,
        gl_context*        gl) {

        static const auto& cfg = config_instance();

        assert(view);
        assert(gl);

        // the same projection and starting camera as the game
        renderer_viewport_init (view->viewport);
        renderer_camera_init   (view->camera);

        view->h_map            = INVALID_HANDLE;
        view->is_requested     = false;
        view->is_grid_visible  = false;
        view->requested_width  = 0;
        view->requested_height = 0;
        view->clear_color.r    = 0.08f;
        view->clear_color.g    = 0.08f;
        view->clear_color.b    = 0.10f;
        view->clear_color.a    = 1.00f;

        // create gl objects
        view->gl.framebuffer   = gl_framebuffer_create  (gl);
        view->gl.texture       = gl_texture_create      (gl);
        view->gl.depth_stencil = gl_renderbuffer_create (gl);

        // the view is resized to fit whatever requests it
        renderer_map_view_resize(view, gl, cfg.window_start_width, cfg.window_start_height);

        bool gl_ok = true;
        gl_ok &= gl_framebuffer_attach_color_texture (gl, view->gl.framebuffer, view->gl.texture);
        gl_ok &= gl_framebuffer_attach_depth_stencil (gl, view->gl.framebuffer, view->gl.depth_stencil);
        gl_ok &= gl_framebuffer_is_complete          (gl, view->gl.framebuffer);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_map_view_request(
        renderer_map_view* view,
        const hnd_map      h_map,
        const u32          width,
        const u32          height) {

        assert(view);
        if (width == 0 || height == 0) return;

        view->h_map            = h_map;
        view->is_requested     = true;
        view->requested_width  = width;
        view->requested_height = height;
    }

    IFB_INTERNAL void
    renderer_map_view_get_view_proj(
        renderer_map_view* view,
        mat4&              xform) {

        assert(view);

        mat4 proj;
        mat4 cam;
        renderer_viewport_get_xform (view->viewport, proj);
        renderer_camera_get_xform   (view->camera,   cam);
        xform = mat4_multiply(proj, cam);
    }

    // this reuses the tile buffer, so it has to go after the game's map is drawn
    // the game's map is packed into it again next frame
    IFB_INTERNAL void
    renderer_map_view_draw(
        renderer_map_view*    view,
        gl_context*           gl,
        render_buffers*       buffers,
        renderer_tile_shader* shdr_tile,
        renderer_grid_shader* shdr_grid) {

        assert(view);
        assert(gl);
        assert(buffers);
        assert(shdr_tile);
        assert(shdr_grid);

        // the view only draws on the frames it is requested
        if (!view->is_requested) return;
        view->is_requested = false;

        if (view->width != view->requested_width || view->height != view->requested_height) {
            renderer_map_view_resize(view, gl, view->requested_width, view->requested_height);
        }

        mat4 view_proj_xform;
        renderer_map_view_get_view_proj(view, view_proj_xform);

        // draw to the texture, the window clear color is put back after
        color_rgba_f32 window_clear_color = gl->clear_color;
        gl_context_set_framebuffer     (gl, view->gl.framebuffer);
        gl_context_update_viewport     (gl, 0, 0, view->width, view->height);
        gl_context_set_clear_color_hex (gl, view->clear_color);
        gl_context_clear_viewport      (gl);
        gl_context_set_clear_color_hex (gl, window_clear_color);

        // map -> tile buffer -> shader
        auto* tile_buf_vtx = buffers->tile_vertices;
        tile_buf_vtx->size = 0;
        if (view->h_map != INVALID_HANDLE) {
            tile_buf_vtx->size = map_render_buffer(
                tile_buf_vtx->data,
                tile_buf_vtx->capacity,
                view->h_map
            );
            assert(tile_buf_vtx->size <= tile_buf_vtx->capacity);
        }
        renderer_tile_draw(shdr_tile, gl, buffers, view->h_map, view_proj_xform);

        if (view->is_grid_visible) {
            renderer_grid_draw(shdr_grid, gl, buffers, view_proj_xform);
        }

        // draw to the window again
        gl_context_set_framebuffer_default(gl);
    }
};
