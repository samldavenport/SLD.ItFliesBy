#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map) {

        const auto& config = ifb_config_instance();

        // stack memory
        eng_stack* stack = eng_stack_init(mem_map);
        assert(stack);

        // stack allocate context
        _eng_context = eng_stack_push_context(stack);
        assert(_eng_context);
        _eng_context->stack   = stack;
        _eng_context->mem_map = mem_map;

        // stack allocate remanining context data
        _eng_context->system    = eng_stack_push_system_info           (stack);
        _eng_context->file_mngr = eng_stack_push_and_init_file_manager (stack, config.file_count); 

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
        const ifb_config& global_cfg = ifb_config_instance();
        pfm_window_config window_cfg;
        window_cfg.title            = (char*)&global_cfg.window_title[0];
        window_cfg.init_dims.width  = global_cfg.window_start_width;
        window_cfg.init_dims.height = global_cfg.window_start_height;
        window_cfg.init_dims.x      = (system->monitor.primary.pixel_width  / 2) - (window_cfg.init_dims.width  / 2); 
        window_cfg.init_dims.y      = (system->monitor.primary.pixel_height / 2) - (window_cfg.init_dims.height / 2); 
        pfm_window_open(&window_cfg);

        // initialize opengl
        gl_context gl;
        pfm_graphics_init_opengl(&gl);

        // file manager
        const u32 file_granularity = size_kilobytes(64);
        file_manager_startup(
            _eng_context->file_mngr,
            mem_map->files.size,
            file_granularity,
            mem_map->files.ptr
        );        

        const file_handle vertex_file_hnd    = file_ro_open_existing (_eng_context->file_mngr, "..\\..\\..\\assets\\shaders\\quad-shader-vertex.glsl");
        const u32         vertex_file_size   = file_get_size         (_eng_context->file_mngr, vertex_file_hnd); 
        const cchar8*     vertex_data        = file_read             (_eng_context->file_mngr, vertex_file_hnd, vertex_file_size);
        
        const file_handle fragment_file_hnd  = file_ro_open_existing (_eng_context->file_mngr, "..\\..\\..\\assets\\shaders\\quad-shader-fragment.glsl");
        const u32         fragment_file_size = file_get_size         (_eng_context->file_mngr, fragment_file_hnd);
        const cchar8*     fragment_data      = file_read             (_eng_context->file_mngr, fragment_file_hnd, fragment_file_size); 

        // create and compile shaders
        const gl_shader  shader_stage_vertex   = gl_shader_stage_create_vertex   (&gl);
        const gl_shader  shader_stage_fragment = gl_shader_stage_create_fragment (&gl);
        bool             did_compile           = true;
        did_compile &= gl_shader_stage_compile_from_source(&gl, shader_stage_vertex,   (const byte*)vertex_data,   vertex_file_size);
        did_compile &= gl_shader_stage_compile_from_source(&gl, shader_stage_fragment, (const byte*)fragment_data, fragment_file_size);
        assert(did_compile);

        // create and link shader program
        const gl_program shader_program = gl_shader_program_create(&gl); 
        bool did_link = true;
        did_link &= gl_shader_program_attach_stage (&gl, shader_program, shader_stage_vertex);
        did_link &= gl_shader_program_attach_stage (&gl, shader_program, shader_stage_fragment);
        did_link &= gl_shader_program_link         (&gl, shader_program);
        assert(did_link);

        // destroy shaders
        gl_shader_program_destroy (&gl, shader_program);
        gl_shader_stage_destroy   (&gl, shader_stage_vertex);
        gl_shader_stage_destroy   (&gl, shader_stage_fragment);
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