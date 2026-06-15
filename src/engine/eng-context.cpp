#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    static const f32 test_vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map) {

        const auto& config = config_instance();

        // stack memory
        eng_stack* stack = eng_stack_init(mem_map);
        assert(stack);

        // stack allocate and initialize context
        _eng_context            = eng_stack_push_context               (stack);
        _eng_context->system    = eng_stack_push_system_info           (stack);
        _eng_context->file_mngr = eng_stack_push_and_init_file_manager (stack, config.file_count); 
        _eng_context->renderer  = eng_stack_push_and_init_renderer     (stack);
        _eng_context->stack     = stack;
        _eng_context->mem_map   = mem_map;

        return(_eng_context);
    }

    IFB_ENGINE_API void
    eng_context_startup(
        void) {

        const eng_mem_map* mem_map = _eng_context->mem_map;
        eng_system_info*   system  = _eng_context->system;

        // monitor info
        system->monitor.count = pfm_monitor_count();
        pfm_monitor_get_info         (0, &system->monitor.primary);
        pfm_monitor_get_working_area (system->monitor.working_area);

        // open window
        const ifb_config& global_cfg = config_instance();
        pfm_window_config window_cfg;
        window_cfg.title            = (char*)&global_cfg.window_title[0];
        window_cfg.init_dims.width  = global_cfg.window_start_width;
        window_cfg.init_dims.height = global_cfg.window_start_height;
        window_cfg.init_dims.x      = (system->monitor.primary.pixel_width  / 2) - (window_cfg.init_dims.width  / 2); 
        window_cfg.init_dims.y      = (system->monitor.primary.pixel_height / 2) - (window_cfg.init_dims.height / 2); 
        pfm_window_open(&window_cfg);

        // file manager
        const u32 file_granularity = size_kilobytes(64);
        file_manager_startup(
            _eng_context->file_mngr,
            mem_map->files.size,
            file_granularity,
            mem_map->files.ptr
        );        

        // shader source
        shader_source vtx_src;
        shader_source frg_src;
        shader_source triangle_shdr_src_vtx;
        shader_source triangle_shdr_src_frg;
        const file_handle quad_vtx_file_hnd     = file_ro_open_existing (_eng_context->file_mngr, "quad-shader-vertex.glsl");
        const file_handle quad_frg_file_hnd     = file_ro_open_existing (_eng_context->file_mngr, "quad-shader-fragment.glsl");
        const file_handle triangle_vtx_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "hello-triangle-shader-vertex.glsl");
        const file_handle triangle_frg_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "hello-triangle-shader-fragment.glsl");
        vtx_src.size                            = file_get_size         (_eng_context->file_mngr, quad_vtx_file_hnd); 
        frg_src.size                            = file_get_size         (_eng_context->file_mngr, quad_frg_file_hnd);
        vtx_src.data                            = file_read             (_eng_context->file_mngr, quad_vtx_file_hnd, vtx_src.size);
        frg_src.data                            = file_read             (_eng_context->file_mngr, quad_frg_file_hnd, frg_src.size); 
        triangle_shdr_src_vtx.size              = file_get_size         (_eng_context->file_mngr, triangle_vtx_file_hnd); 
        triangle_shdr_src_frg.size              = file_get_size         (_eng_context->file_mngr, triangle_frg_file_hnd);
        triangle_shdr_src_vtx.data              = file_read             (_eng_context->file_mngr, triangle_vtx_file_hnd, triangle_shdr_src_vtx.size);
        triangle_shdr_src_frg.data              = file_read             (_eng_context->file_mngr, triangle_frg_file_hnd, triangle_shdr_src_frg.size); 

        // renderer
        memory mem_rndr;
        mem_rndr.ptr  = mem_map->rendering.ptr;
        mem_rndr.size = mem_map->rendering.size;
        renderer_context_startup            (_eng_context->renderer, mem_rndr);
        renderer_quad_shader_init           (_eng_context->renderer, vtx_src,               frg_src);
        // renderer_hello_triangle_shader_init (_eng_context->renderer, triangle_shdr_src_vtx, triangle_shdr_src_frg);
        renderer_hello_quad_shader_init(_eng_context->renderer, triangle_shdr_src_vtx, triangle_shdr_src_frg);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        while(true) {

            //TODO(SAM): pass the opengl context to the platform
            
            // start new frame
            pfm_window_frame_start   ();
            pfm_window_process_events();
            
            renderer_hello_quad_draw(_eng_context->renderer);
            // renderer_hello_triangle_draw(_eng_context->renderer);
            // quad q;
            // q.color  = color_rgba_u32(0xFF0000FF);
            // q.height = 0.5f;
            // q.width  = 0.5f;
            // q.scale  = 1.0f;
            // q.pos.x  = 0.0f;
            // q.pos.y  = 0.0f;
            // q.pos.z  = 0.0f;

            // renderer_quad_push     (_eng_context->renderer, &q);
            // renderer_quad_draw_all (_eng_context->renderer);

            // render frame
            pfm_window_frame_render   ();
        }
    }
    
    IFB_ENGINE_API void
    eng_context_shutdown(
        void) {

    }
};