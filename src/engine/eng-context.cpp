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
        renderer_context_startup          (_eng_context->renderer, mem_rndr);
        // renderer_quad_shader_init (_eng_context->renderer, vtx_src, frg_src);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        bool gl_ok = true;

        const file_handle vtx_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "hello-triangle-shader-vertex.glsl");
        const file_handle frg_file_hnd = file_ro_open_existing (_eng_context->file_mngr, "hello-triangle-shader-fragment.glsl");
        const u32         vtx_src_size = file_get_size         (_eng_context->file_mngr, vtx_file_hnd); 
        const u32         frg_src_size = file_get_size         (_eng_context->file_mngr, frg_file_hnd);
        const char*       vtx_src_data = file_read             (_eng_context->file_mngr, vtx_file_hnd, vtx_src_size);
        const char*       frg_src_data = file_read             (_eng_context->file_mngr, frg_file_hnd, frg_src_size); 

        const u32 test_program  = gl_shader_program_create        (_eng_context->renderer->gl);
        const u32 test_shdr_vtx = gl_shader_stage_create_vertex   (_eng_context->renderer->gl);
        const u32 test_shdr_frg = gl_shader_stage_create_fragment (_eng_context->renderer->gl);
        const u32 test_vao      = gl_vertex_create                (_eng_context->renderer->gl);
        const u32 test_vbo      = gl_buffer_create                (_eng_context->renderer->gl); 

        gl_ok &= gl_shader_stage_compile_from_source (_eng_context->renderer->gl, test_shdr_vtx, vtx_src_data, vtx_src_size);
        gl_ok &= gl_shader_stage_compile_from_source (_eng_context->renderer->gl, test_shdr_frg, frg_src_data, frg_src_size);
        gl_ok &= gl_shader_program_attach_stage      (_eng_context->renderer->gl, test_program,  test_shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (_eng_context->renderer->gl, test_program,  test_shdr_frg);
        gl_ok &= gl_shader_program_link              (_eng_context->renderer->gl, test_program);
        gl_shader_stage_destroy                      (_eng_context->renderer->gl, test_shdr_vtx);
        gl_shader_stage_destroy                      (_eng_context->renderer->gl, test_shdr_frg);
        assert(gl_ok);

        gl_ok&= gl_context_set_vertex_object (_eng_context->renderer->gl, test_vao);
        gl_ok&= gl_context_set_buffer_vertex (_eng_context->renderer->gl, test_vbo);
        gl_ok&= gl_buffer_set_vertex_data    (_eng_context->renderer->gl, test_vbo, (const byte*)test_vertices, sizeof(test_vertices));
        gl_ok&= gl_vertex_add_attribute_f32x3(_eng_context->renderer->gl, test_vao, (3 * sizeof(float)), 0, NULL);
        assert(gl_ok);


        while(true) {

            //TODO(SAM): pass the opengl context to the platform
            
            // start new frame
            pfm_window_frame_start   ();
            pfm_window_process_events();
            
            gl_context_set_shader_program (_eng_context->renderer->gl, test_program);
            gl_context_set_vertex_object  (_eng_context->renderer->gl, test_vao);
            gl_context_draw_vertices      (_eng_context->renderer->gl, 3);

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