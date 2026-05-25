#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    static eng_context* _eng_ctx;    

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map) {

        const u32 size_struct_ctx   = sizeof(eng_context);
        const u32 size_struct_stack = sizeof(eng_stack);

        zero_memory(mem_map->stack.ptr, mem_map->stack.size);

        // context
        _eng_ctx = (eng_context*)mem_map->stack.ptr;

        // stack
        auto stack      = (eng_stack*)(((byte*)_eng_ctx) + size_struct_ctx);
        stack->start    =      (byte*)(((byte*)stack)    + size_struct_stack);
        stack->size     = mem_map->stack.size - (size_struct_ctx + size_struct_stack);
        stack->position = 0;
        _eng_ctx->stack = stack;

        // system info
        _eng_ctx->system = eng_stack_push_struct<eng_system_info>();

        return(_eng_ctx);
    }

    IFB_ENGINE_API void
    eng_context_startup(
        void) {

        eng_system_info* system = _eng_ctx->system;
        // monitor info
        system->monitor.count = pfm_monitor_count();
        pfm_monitor_get_info         (0, &system->monitor.primary);
        pfm_monitor_get_working_area (system->monitor.working_area);

        const ifb_config& global_cfg = ifb_config_instance();
        pfm_window_config window_cfg;
        window_cfg.title            = (char*)&global_cfg.window_title[0];
        window_cfg.init_dims.width  = global_cfg.window_start_width;
        window_cfg.init_dims.height = global_cfg.window_start_height;
        window_cfg.init_dims.x      = (system->monitor.primary.pixel_width  / 2) - (window_cfg.init_dims.width  / 2); 
        window_cfg.init_dims.y      = (system->monitor.primary.pixel_height / 2) - (window_cfg.init_dims.height / 2); 
        pfm_window_open(&window_cfg);

        gl_context gl;
        pfm_graphics_init_opengl(&gl);

        pfm_file_config file_config_vertex;
        file_config_vertex.path         = "..\\..\\..\\assets\\shaders\\quad-shader-vertex.glsl";
        file_config_vertex.mode         = pfm_file_mode_e_open_existing;
        file_config_vertex.access_flags = pfm_file_access_flag_e_read;
        file_config_vertex.share_flags  = pfm_file_share_flag_e_read;
        file_config_vertex.is_async     = false;

        pfm_file_config file_config_fragment;
        file_config_fragment.path         = "..\\..\\..\\assets\\shaders\\quad-shader-fragment.glsl";
        file_config_fragment.mode         = pfm_file_mode_e_open_existing;
        file_config_fragment.access_flags = pfm_file_access_flag_e_read;
        file_config_fragment.share_flags  = pfm_file_share_flag_e_read;
        file_config_fragment.is_async     = false;

        const pfm_file_handle file_hnd_vertex    = pfm_file_open (&file_config_vertex);
        const pfm_file_handle file_hnd_fragment  = pfm_file_open (&file_config_fragment);
        const u32             file_size_vertex   = pfm_file_size (file_hnd_vertex);
        const u32             file_size_fragment = pfm_file_size (file_hnd_fragment);

        const u32 data_size = (file_size_vertex + file_size_fragment);
        void*     data_ptr  = malloc(data_size);
        assert(data_size != 0 && data_ptr != NULL);

        pfm_file_buffer file_buffer_vertex;
        file_buffer_vertex.cursor = 0;
        file_buffer_vertex.data   = (byte*)data_ptr;
        file_buffer_vertex.length = 0;
        file_buffer_vertex.offset = 0;
        file_buffer_vertex.size   = file_size_vertex;        
        pfm_file_read(file_hnd_vertex, &file_buffer_vertex);

        pfm_file_buffer file_buffer_fragment;
        file_buffer_fragment.cursor = 0;
        file_buffer_fragment.data   = (byte*)data_ptr + file_size_vertex;
        file_buffer_fragment.length = 0;
        file_buffer_fragment.offset = 0;
        file_buffer_fragment.size   = file_size_fragment;
        pfm_file_read(file_hnd_fragment, &file_buffer_fragment);



        free(data_ptr); 
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

    IFB_ENG_INTERNAL byte*
    eng_stack_push_data(
        const u32 size) {

        assert(_eng_ctx && _eng_ctx->stack);

        eng_stack* stack = _eng_ctx->stack;        

        const u32 new_position = (stack->position + size);
        assert(new_position <= stack->size);

        byte* data = &stack->start[stack->position];

        stack->position = new_position;

        return(data);
    }
    
    template<typename t>
    IFB_ENG_INTERNAL t* 
    eng_stack_push_struct(
        const u32 count) {

        assert(count != 0);

        const u32 push_size = count * sizeof(t);

        t* push = (t*)eng_stack_push_data(push_size);
        assert(push);

        return(push);
    }

};