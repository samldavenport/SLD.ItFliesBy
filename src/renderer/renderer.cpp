#pragma once

#include "renderer.hpp"
#include "memory-reservation.cpp"
#include "renderer-quad.cpp" 
#include "renderer-camera.cpp"
#include "renderer-projection.cpp"
#include "renderer-direction-gizmo.cpp"
#include "renderer-grid.cpp"
#include "renderer-tile.cpp"
#include "eng-internal.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL renderer_context*
    renderer_context_create(
        void) {

        const auto& cfg         = config_instance();
        const u32   block_count = (cfg.memory_size_rendering / cfg.renderer_mem_granularity);

        auto rndr      = global_alloc<renderer_context> ();
        auto gl        = global_alloc<gl_context>       ();
        auto block_ids = global_alloc<u32>              (block_count); 
        assert(
            rndr      != NULL &&
            gl        != NULL &&
            block_ids != NULL            
        );

        _renderer_ctx     = rndr;
        _renderer_ctx->gl = gl;

        return(_renderer_ctx);
    }

    IFB_INTERNAL void
    renderer_context_startup(
        reservation* res) {

        const auto& cfg = config_instance();

        assert(_renderer_ctx != NULL);
        assert(res);

        // create the stack
        _renderer_ctx->memory.stack = reservation_push_stack_all(res);

        // NOTE(SAM): the renderer doesn't need to initialize the opengl context
        // we can pass the context to the function and use it that way
        // same for imgui, it can be initialized externally

        // create shaders
        renderer_grid_shader_create();


        // set global up
        _renderer_ctx->global_up = { 0.0f, 1.0f, 0.0f };

        // initialize opengl and imgui
        pfm_graphics_init_opengl(_renderer_ctx->gl);
        pfm_graphics_init_imgui();

        // create shaders
        renderer_quad_shader_create();
        renderer_tile_shader_create();
        renderer_direciton_gizmo_shader_create();

        // intialize camera
        renderer_projection_init();
        renderer_camera_init();
        renderer_projection_set_viewport(cfg.window_start_width, cfg.window_start_height);

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
        renderer_quad_shader_init            (file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (file_src_dir_giz_vert, file_src_dir_giz_frag);
        renderer_grid_shader_init            (file_src_grid_vert,    file_src_grid_frag);
        renderer_tile_shader_init            (file_src_tile_vert,    file_src_tile_frag);

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
    renderer_context_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void*
    renderer_context_memory_alloc(
        const u32 size) {

        assert(size != 0);

        void* mem = stack_push(_renderer_ctx->memory.stack, size);

        return(mem);
    }

    IFB_INTERNAL mat4
    renderer_context_view_projection_xform(
        void) {

        // calculate view and projection matrices
        static mat4 proj = mat4_identity();
        static mat4 view = mat4_identity();
        proj = renderer_projection_xform ();
        view = renderer_camera_xform     ();
       
        // calculate view projection
        const mat4 view_proj = mat4_multiply(proj, view);
        return(view_proj);
    }
    
    IFB_INTERNAL void
    renderer_context_draw_buffers(
        void) {

        const mat4 view_proj_xform = renderer_context_view_projection_xform(); 

        //renderer_direction_gizmo_draw (view_proj_xform);
        renderer_tile_draw            (view_proj_xform);
        renderer_grid_draw            (view_proj_xform);
        renderer_quad_draw            (view_proj_xform);
    }
};
          
