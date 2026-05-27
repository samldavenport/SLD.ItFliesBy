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



        // vertex shader file config
        const file_handle hnd = file_ro_open_existing(
            _eng_context->file_mngr,
            "..\\..\\..\\assets\\shaders\\quad-shader-vertex.glsl"
        );
        
        pfm_file_config file_config_vertex;
        file_config_vertex.path         = "..\\..\\..\\assets\\shaders\\quad-shader-vertex.glsl";
        file_config_vertex.mode         = pfm_file_mode_e_open_existing;
        file_config_vertex.access_flags = pfm_file_access_flag_e_read;
        file_config_vertex.share_flags  = pfm_file_share_flag_e_read;
        file_config_vertex.is_async     = false;

        // fragment shader file config
        pfm_file_config file_config_fragment;
        file_config_fragment.path         = "..\\..\\..\\assets\\shaders\\quad-shader-fragment.glsl";
        file_config_fragment.mode         = pfm_file_mode_e_open_existing;
        file_config_fragment.access_flags = pfm_file_access_flag_e_read;
        file_config_fragment.share_flags  = pfm_file_share_flag_e_read;
        file_config_fragment.is_async     = false;

        // open the files ad get sizes
        const pfm_file_handle file_hnd_vertex    = pfm_file_open (&file_config_vertex);
        const pfm_file_handle file_hnd_fragment  = pfm_file_open (&file_config_fragment);
        const u32             file_size_vertex   = pfm_file_size (file_hnd_vertex);
        const u32             file_size_fragment = pfm_file_size (file_hnd_fragment);

        // allocate memory
        const u32 data_size = (file_size_vertex + file_size_fragment);
        void*     data_ptr  = malloc(data_size);
        assert(data_size != 0 && data_ptr != NULL);

        // read vertex shader
        pfm_file_buffer file_buffer_vertex;
        file_buffer_vertex.cursor = 0;
        file_buffer_vertex.data   = (byte*)data_ptr;
        file_buffer_vertex.length = 0;
        file_buffer_vertex.offset = 0;
        file_buffer_vertex.size   = file_size_vertex;        
        pfm_file_read(file_hnd_vertex, &file_buffer_vertex);

        // read fragment buffer
        pfm_file_buffer file_buffer_fragment;
        file_buffer_fragment.cursor = 0;
        file_buffer_fragment.data   = (byte*)data_ptr + file_size_vertex;
        file_buffer_fragment.length = 0;
        file_buffer_fragment.offset = 0;
        file_buffer_fragment.size   = file_size_fragment;
        pfm_file_read(file_hnd_fragment, &file_buffer_fragment);

        // create and compile shaders
        const gl_shader  shader_stage_vertex   = gl_shader_stage_create_vertex   (&gl);
        const gl_shader  shader_stage_fragment = gl_shader_stage_create_fragment (&gl);
        bool             did_compile           = true;
        did_compile &= gl_shader_stage_compile_from_source(&gl, shader_stage_vertex,   file_buffer_vertex.data,   file_buffer_vertex.length);
        did_compile &= gl_shader_stage_compile_from_source(&gl, shader_stage_fragment, file_buffer_fragment.data, file_buffer_fragment.length);
        assert(did_compile);

        // create and link shader program
        const gl_program shader_program = gl_shader_program_create(&gl); 
        bool did_link = true;
        did_link &= gl_shader_program_attach_stage (&gl, shader_program, shader_stage_vertex);
        did_link &= gl_shader_program_attach_stage (&gl, shader_program, shader_stage_fragment);
        did_link &= gl_shader_program_link         (&gl, shader_program);
        assert(did_link);

        // destroy shaders and free memory
        pfm_file_close            (file_hnd_vertex);
        pfm_file_close            (file_hnd_fragment);
        gl_shader_program_destroy (&gl, shader_program);
        gl_shader_stage_destroy   (&gl, shader_stage_vertex);
        gl_shader_stage_destroy   (&gl, shader_stage_fragment);
        free                      (data_ptr); 
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