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
        _eng_context            = eng_stack_push_context           (stack);
        _eng_context->system    = eng_stack_push_system_info       (stack);
        _eng_context->renderer  = eng_stack_push_and_init_renderer (stack);
        _eng_context->mngrs     = eng_stack_push_and_init_managers (stack); 
        _eng_context->gui       = eng_stack_push_and_init_gui      (stack);
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

        eng_managers* mngrs = _eng_context->mngrs;

        // file manager
        const u32 file_granularity = size_kilobytes(64);
        file_manager_startup(
            mngrs->file,
            mem_map->files.size,
            file_granularity,
            mem_map->files.ptr
        );        

        // entity manager
        memory entity_mem;
        entity_mem.size = mem_map->entities.size;
        entity_mem.ptr  = mem_map->entities.ptr;
        entity_manager_startup(mngrs->entity, entity_mem);

        // initialize the renderer
        memory mem_rndr;
        mem_rndr.ptr  = mem_map->rendering.ptr;
        mem_rndr.size = mem_map->rendering.size;
        renderer_context_startup        (_eng_context->renderer, mem_rndr);

        // open shader files
        const file_handle file_hnd_quad_vert    = file_ro_open_existing (mngrs->file, "quad-shader-vertex.glsl");
        const file_handle file_hnd_quad_frag    = file_ro_open_existing (mngrs->file, "quad-shader-fragment.glsl");
        const file_handle file_hnd_dir_giz_vert = file_ro_open_existing (mngrs->file, "direction-gizmo-shader-vert.glsl");
        const file_handle file_hnd_dir_giz_frag = file_ro_open_existing (mngrs->file, "direction-gizmo-shader-frag.glsl");

        // read quad shaders        
        shader_source file_src_quad_vert;
        shader_source file_src_quad_frag;
        file_src_quad_vert.size = file_get_size (mngrs->file, file_hnd_quad_vert); 
        file_src_quad_vert.data = file_read     (mngrs->file, file_hnd_quad_vert, file_src_quad_vert.size);
        file_src_quad_frag.size = file_get_size (mngrs->file, file_hnd_quad_frag);
        file_src_quad_frag.data = file_read     (mngrs->file, file_hnd_quad_frag, file_src_quad_frag.size); 
        
        // read direction gizmo shaders
        shader_source file_src_dir_giz_vert;
        shader_source file_src_dir_giz_frag;
        file_src_dir_giz_vert.size = file_get_size (mngrs->file, file_hnd_dir_giz_vert); 
        file_src_dir_giz_vert.data = file_read     (mngrs->file, file_hnd_dir_giz_vert, file_src_dir_giz_vert.size);
        file_src_dir_giz_frag.size = file_get_size (mngrs->file, file_hnd_dir_giz_frag);
        file_src_dir_giz_frag.data = file_read     (mngrs->file, file_hnd_dir_giz_frag, file_src_dir_giz_frag.size); 

        // initialize shaders
        renderer_quad_shader_init            (_eng_context->renderer, file_src_quad_vert,    file_src_quad_frag);
        renderer_hello_quad_shader_init      (_eng_context->renderer, file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (_eng_context->renderer, file_src_dir_giz_vert, file_src_dir_giz_frag);

        // start the gui
        memory gui_mem;
        gui_mem.ptr  = mem_map->gui.ptr;
        gui_mem.size = mem_map->gui.size;
        gui_startup(_eng_context->gui, gui_mem);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        quad test_quad;
        test_quad.color.hex         = 0xFF0000FF;
        test_quad.position.x        = 0.0f;
        test_quad.position.y        = 0.0f;
        test_quad.position.z        = 0.0f;
        test_quad.dimensions.width  = 0.2f;
        test_quad.dimensions.height = 0.2f;

        while(true) {

            //TODO(SAM): pass the opengl context to the platform
            // start new frame
            pfm_window_frame_start   ();
            pfm_window_process_events();

            // push and render quad
            renderer_quad_push            (_eng_context->renderer, &test_quad, 1);
            renderer_quad_draw            (_eng_context->renderer);
            renderer_hello_quad_draw      (_eng_context->renderer);
            renderer_direction_gizmo_draw (_eng_context->renderer);

            gui_render(_eng_context->gui);

            // render frame
            pfm_window_frame_render();

            // check if quit received
            const bool quit = pfm_window_quit_received();
            if (quit) break;
        }
    }
    
    IFB_ENGINE_API void
    eng_context_shutdown(
        void) {

    }
};