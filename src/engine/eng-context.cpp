#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {



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

        // renderer
        shader_source vtx_src;
        shader_source frg_src;
        const file_handle vtx_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "..\\..\\..\\assets\\shaders\\quad-shader-vertex.glsl");
        const file_handle frg_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "..\\..\\..\\assets\\shaders\\quad-shader-fragment.glsl");
        vtx_src.size = file_get_size (_eng_context->file_mngr, vtx_file_hnd); 
        vtx_src.data = file_read     (_eng_context->file_mngr, vtx_file_hnd, vtx_src.size);
        frg_src.size = file_get_size (_eng_context->file_mngr, frg_file_hnd);
        frg_src.data = file_read     (_eng_context->file_mngr, frg_file_hnd, frg_src.size); 
        memory mem_rndr;
        mem_rndr.ptr  = mem_map->rendering.ptr;
        mem_rndr.size = mem_map->rendering.size;
        renderer_startup          (_eng_context->renderer, mem_rndr);
        renderer_init_quad_shader (_eng_context->renderer, vtx_src, frg_src);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        while(true) {

            //TODO(SAM): pass the opengl context to the platform
            pfm_window_frame_start    ();
            pfm_window_process_events ();
            pfm_window_frame_render   ();
        }
    }
    
    IFB_ENGINE_API void
    eng_context_shutdown(
        void) {

    }


};