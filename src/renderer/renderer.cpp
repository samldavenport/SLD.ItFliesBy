#pragma once

#include "renderer.hpp"
#include "ifb-collections.hpp"
#include "memory-reservation.cpp"
#include "renderer-internal.hpp"
#include "render-buffers.cpp"
#include "renderer-draw-lists.cpp"
#include "renderer-quad.cpp"
#include "renderer-camera.cpp"
#include "renderer-viewport.cpp"
#include "renderer-direction-gizmo.cpp"
#include "renderer-grid.cpp"
#include "renderer-tile.cpp"
#include "eng-internal.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math.hpp"
#include "sld-opengl.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct renderer_memory {
        stack* stack;
    };

    struct renderer {
        gl_context*        gl;
        renderer_memory    memory;
        renderer_camera*   cam;
        renderer_viewport* viewport;
        vec3               global_up;
        render_lists*      lists;
        render_buffers*    buffers;
        struct {
            renderer_quad_shader*            quad;
            renderer_direction_gizmo_shader* direction_gizmo;
            renderer_grid_shader*            grid; 
            renderer_tile_shader*            tile;
        } shader;
    };
    
    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL renderer*
    renderer_create(
        void) {

        const auto& cfg         = config_instance();
        const u32   block_count = (cfg.memory_size_rendering / cfg.renderer_mem_granularity);

        auto rndr        = global_alloc<renderer> ();
        auto gl          = global_alloc<gl_context>       ();
        auto block_ids   = global_alloc<u32>              (block_count); 
        auto camera      = renderer_camera_create();
        auto viewport    = renderer_viewport_create();
        auto lists       = render_lists_create();
        auto buffers     = render_buffers_create();
        assert(
            rndr       != NULL &&
            gl         != NULL &&
            block_ids  != NULL &&
            camera     != NULL &&
            viewport   != NULL &&
            lists      != NULL &&
            buffers    != NULL
        );

        _renderer                         = rndr;
        _renderer->gl                     = gl;
        _renderer->cam                    = camera;
        _renderer->viewport               = viewport;
        _renderer->lists                  = lists;
        _renderer->buffers                = buffers;

        return(_renderer);
    }

    IFB_INTERNAL void
    renderer_startup(
        reservation* res) {

        const auto& cfg = config_instance();

        assert(_renderer != NULL);
        assert(res);

        // create the stack
        _renderer->memory.stack = reservation_push_stack_all(res);

        // NOTE(SAM): the renderer doesn't need to initialize the opengl context
        // we can pass the context to the function and use it that way
        // same for imgui, it can be initialized externally

        // set global up
        _renderer->global_up = { 0.0f, 1.0f, 0.0f };

        // initialize opengl and imgui
        pfm_graphics_init_opengl(_renderer->gl);
        pfm_graphics_init_imgui();

        // initialize the lists and buffers
        // the shaders size their gl buffers from the render buffers,
        // so this needs to happen first
        render_lists_init             (_renderer->lists);
        render_buffers_alloc_and_init (_renderer->buffers);

        // create shaders
        _renderer->shader.quad            = renderer_quad_shader_create();
        _renderer->shader.grid            = renderer_grid_shader_create();
        _renderer->shader.tile            = renderer_tile_shader_create();
        _renderer->shader.direction_gizmo = renderer_direciton_gizmo_shader_create();

        // intialize camera
        renderer_viewport_init (_renderer->viewport); 
        renderer_camera_init   (_renderer->cam);
        renderer_viewport_set_dimensions(_renderer->viewport, _renderer->gl, cfg.window_start_width, cfg.window_start_height);

        // open shader files
        const hnd_file file_hnd_quad_vert    = file_ro_open_existing ("quad-shader-vertex.glsl");
        const hnd_file file_hnd_quad_frag    = file_ro_open_existing ("quad-shader-fragment.glsl");
        const hnd_file file_hnd_dir_giz_vert = file_ro_open_existing ("direction-gizmo-shader-vert.glsl");
        const hnd_file file_hnd_dir_giz_frag = file_ro_open_existing ("direction-gizmo-shader-frag.glsl");
        const hnd_file file_hnd_grid_vert    = file_ro_open_existing ("grid-vert.glsl");
        const hnd_file file_hnd_grid_frag    = file_ro_open_existing ("grid-frag.glsl");
        const hnd_file file_hnd_tile_vert    = file_ro_open_existing ("tile-vert.glsl");
        const hnd_file file_hnd_tile_frag    = file_ro_open_existing ("tile-frag.glsl");

        // read quad shaders        
        renderer_shader_source file_src_quad_vert;
        renderer_shader_source file_src_quad_frag;
        file_src_quad_vert.size = file_get_size (file_hnd_quad_vert); 
        file_src_quad_vert.data = file_read     (file_hnd_quad_vert, file_src_quad_vert.size);
        file_src_quad_frag.size = file_get_size (file_hnd_quad_frag);
        file_src_quad_frag.data = file_read     (file_hnd_quad_frag, file_src_quad_frag.size); 
        
        // read direction gizmo shaders
        renderer_shader_source file_src_dir_giz_vert;
        renderer_shader_source file_src_dir_giz_frag;
        file_src_dir_giz_vert.size = file_get_size (file_hnd_dir_giz_vert); 
        file_src_dir_giz_vert.data = file_read     (file_hnd_dir_giz_vert, file_src_dir_giz_vert.size);
        file_src_dir_giz_frag.size = file_get_size (file_hnd_dir_giz_frag);
        file_src_dir_giz_frag.data = file_read     (file_hnd_dir_giz_frag, file_src_dir_giz_frag.size); 

        // read grid shaders
        renderer_shader_source file_src_grid_vert;
        renderer_shader_source file_src_grid_frag;
        file_src_grid_vert.size = file_get_size (file_hnd_grid_vert); 
        file_src_grid_vert.data = file_read     (file_hnd_grid_vert, file_src_grid_vert.size);
        file_src_grid_frag.size = file_get_size (file_hnd_grid_frag);
        file_src_grid_frag.data = file_read     (file_hnd_grid_frag, file_src_grid_frag.size); 
        
        // read grid shaders
        renderer_shader_source file_src_tile_vert;
        renderer_shader_source file_src_tile_frag;
        file_src_tile_vert.size = file_get_size (file_hnd_tile_vert); 
        file_src_tile_vert.data = file_read     (file_hnd_tile_vert, file_src_tile_vert.size);
        file_src_tile_frag.size = file_get_size (file_hnd_tile_frag);
        file_src_tile_frag.data = file_read     (file_hnd_tile_frag, file_src_tile_frag.size); 
        
        // initialize shaders
        renderer_quad_shader_init            (_renderer->shader.quad,            _renderer->gl, _renderer->buffers, file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (_renderer->shader.direction_gizmo, _renderer->gl,                     file_src_dir_giz_vert, file_src_dir_giz_frag);
        renderer_grid_shader_init            (_renderer->shader.grid,            _renderer->gl, _renderer->buffers, file_src_grid_vert,    file_src_grid_frag);
        renderer_tile_shader_init            (_renderer->shader.tile,            _renderer->gl, _renderer->buffers, file_src_tile_vert,    file_src_tile_frag);

        // close the shader files
        file_close(file_hnd_quad_vert);
        file_close(file_hnd_quad_frag);
        file_close(file_hnd_dir_giz_vert);
        file_close(file_hnd_dir_giz_frag);
        file_close(file_hnd_grid_vert);
        file_close(file_hnd_grid_frag);
        file_close(file_hnd_tile_vert);
        file_close(file_hnd_tile_frag);
    }

    IFB_INTERNAL void
    renderer_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void*
    renderer_memory_alloc(
        const u32 size) {

        assert(size != 0);

        void* mem = stack_push(_renderer->memory.stack, size);

        return(mem);
    }

    IFB_INTERNAL void 
    renderer_view_projection_xform(
        mat4& xform) {

        // calculate view and projection matrices
        static mat4 proj = mat4_identity();
        static mat4 view = mat4_identity();
        renderer_viewport_get_xform (_renderer->viewport, proj);
        renderer_camera_get_xform   (_renderer->cam, view);
       
        // calculate view projection
        xform = mat4_multiply(proj, view);
    }
    
    IFB_INTERNAL void
    renderer_draw_buffers(
        void) {

        auto* gl      = _renderer->gl;
        auto* lists   = _renderer->lists;
        auto* buffers = _renderer->buffers;

        mat4 view_proj_xform;
        renderer_view_projection_xform(view_proj_xform);

        // lists -> buffers
        render_buffers_render_tiles   (buffers, lists);
        render_buffers_render_quads   (buffers, lists, _renderer->cam);

        // buffers -> shaders
        //renderer_direction_gizmo_draw (view_proj_xform);
        renderer_tile_draw            (_renderer->shader.tile, gl, buffers, render_lists_get_map(lists), view_proj_xform);
        renderer_grid_draw            (_renderer->shader.grid, gl, buffers, view_proj_xform);
        renderer_quad_draw            (_renderer->shader.quad, gl, buffers, view_proj_xform);

        // the lists get filled again next frame
        render_lists_reset(lists);
    }

    IFB_INTERNAL entity_list*
    renderer_create_entity_list(
        void) {

        memory mem;
        mem.size = entity_list_mem_req();
        mem.ptr  = renderer_memory_alloc(mem.size);
        if (mem.size == 0 || mem.address == 0) {
            return(NULL);
        }

        entity_list* list = entity_list_memory_create(mem);
        return(list);
    }
    
    IFB_INTERNAL bool 
    renderer_push_quad(
        const entity_id id) {

        assert(id != INVALID_ID);
        assert(_renderer);
        assert(_renderer->lists);

        const bool did_push = render_lists_push_quad(_renderer->lists, id);
return(did_push);
    }
    
    IFB_INTERNAL bool
    renderer_set_map(
        const hnd_map h_map) {

        assert(h_map                      != INVALID_HANDLE);
        assert(_renderer              != NULL);
        assert(_renderer->lists       != NULL);

        const bool did_set = render_lists_set_map(_renderer->lists, h_map);
return(did_set);
    }
    
    //--------------------------------------------------------------------
    // CAMERA METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const vec3&
    renderer_get_camera_origin(
        void) {

        assert(_renderer);
        return(renderer_camera_get_origin(_renderer->cam));
    }

    IFB_INTERNAL const vec3&
    renderer_get_camera_target(
        void) {

        assert(_renderer);
        return(renderer_camera_get_target(_renderer->cam));
    }

    IFB_INTERNAL void
    renderer_get_camera_orientation(
        orientation& o) {

        assert(_renderer);
        renderer_camera_get_orientation(_renderer->cam, o);
    }

    IFB_INTERNAL void 
    renderer_get_camera_xform(
        mat4& xform) {

        assert(_renderer);
        renderer_camera_get_xform(_renderer->cam, xform);
    }

    IFB_INTERNAL void
    renderer_set_camera_origin(
        const vec3& origin) {

        assert(_renderer);
        renderer_camera_set_origin(_renderer->cam, origin);
    }

    IFB_INTERNAL void
    renderer_set_camera_target(
        const vec3& target) {

        assert(_renderer);
        renderer_camera_set_target(_renderer->cam, target);
    }
    
    //--------------------------------------------------------------------
    // VIEWPORT METHODS 
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void
    renderer_set_viewport_dimensions(
        const u32 width,
        const u32 height) {

        assert(width  != 0);
        assert(height != 0);
        assert(_renderer);
        assert(_renderer->viewport);
       
        renderer_viewport_set_dimensions(
            _renderer->viewport,
            _renderer->gl,
            width, height
        );
    }

    IFB_INTERNAL f32
    renderer_get_viewport_aspect_ratio(
        void) {

        assert(_renderer);
        assert(_renderer->viewport);

        const f32 aspect_ratio = renderer_viewport_get_aspect_ratio(_renderer->viewport);
        return(aspect_ratio);
    }

    IFB_INTERNAL void
    renderer_get_viewport_xform(
        mat4& xform) {

        assert(_renderer);
        assert(_renderer->viewport);

        renderer_viewport_get_xform(_renderer->viewport, xform);
    }
};
          
