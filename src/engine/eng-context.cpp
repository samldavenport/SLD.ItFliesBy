#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void eng_context_startup_get_system_info (eng_system_info* sys_info);
    IFB_INLINE void eng_context_startup_open_window     (const config& config, const eng_system_info* sys_info);
    IFB_INLINE void eng_context_startup_file_mngr       (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_entity_mngr     (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_memory_mngr     (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_renderer        (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_component_mngr  (const eng_mem_map* mem_map);

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map) {

        const auto& config = config_instance();

        // stack memory
        global_stack_create_and_init(mem_map);

        // allocate global memory
        auto eng_ctx     = global_alloc<eng_context>      (); 
        auto sys_info    = global_alloc<eng_system_info>  ();
        assert(
            eng_ctx     != NULL &&
            sys_info    != NULL
        );

        // set context properties        
        _eng_context              = eng_ctx;
        _eng_context->mem_map     = mem_map;
        _eng_context->system      = sys_info;  
        _eng_context->renderer    = renderer_context_create(); 
        _eng_context->file_mngr   = file_mngr_create(); 
        _eng_context->entity_mngr = entity_mngr_create(); 
        _eng_context->memory_mngr = memory_mngr_create(); 
        _eng_context->cmpnt_mngr  = component_mngr_create();     
        _eng_context->mem_map     = mem_map;

        assert(
            _eng_context->mem_map     != NULL &&
            _eng_context->system      != NULL &&
            _eng_context->renderer    != NULL &&
            _eng_context->file_mngr   != NULL &&
            _eng_context->entity_mngr != NULL &&
            _eng_context->memory_mngr != NULL &&
            _eng_context->mem_map     != NULL
        );

        return(_eng_context);
    }

    IFB_ENGINE_API void
    eng_context_startup(
        void) {

        const auto&        config   = config_instance();
        const eng_mem_map* mem_map  = _eng_context->mem_map;
        eng_system_info*   system   = _eng_context->system;
        renderer_context*  renderer = _eng_context->renderer;

        eng_context_startup_get_system_info (_eng_context->system);
        eng_context_startup_open_window     (config, system);
        eng_context_startup_file_mngr       (mem_map);
        eng_context_startup_entity_mngr     (mem_map);
        eng_context_startup_memory_mngr     (mem_map);
        eng_context_startup_renderer        (mem_map);
        eng_context_startup_component_mngr  (mem_map);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        const eng_arena_handle img_arena = eng_arena_alloc();
        const eng_file_handle  img_file  = eng_file_ro_open_existing("../../../assets/images/test-sprite.png");
        const image*           img       = eng_image_load_to_arena(img_file, img_arena);

        while(true) {

            //TODO(SAM): pass the opengl context to the platform
            // start new frame
            pfm_window_frame_start   ();
            pfm_window_process_events();

            // render graphics
            renderer_context_update_projection_matrix ();
            renderer_context_update_view_matrix       ();
            renderer_direction_gizmo_draw             ();

            // render gui
            gui_render();

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

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    eng_context_startup_get_system_info(
        eng_system_info* sys_info) {

        // monitor info
        sys_info->monitor.count = pfm_monitor_count();
        pfm_monitor_get_info         (0, &sys_info->monitor.primary);
        pfm_monitor_get_working_area (sys_info->monitor.working_area);
    }
    
    IFB_INLINE void
    eng_context_startup_open_window(
        const config&      config,
        const eng_system_info* sys_info) {

        pfm_window_config window_cfg;
        window_cfg.title            = (char*)&config.window_title[0];
        window_cfg.init_dims.width  = config.window_start_width;
        window_cfg.init_dims.height = config.window_start_height;
        window_cfg.init_dims.x      = (sys_info->monitor.primary.pixel_width  / 2) - (window_cfg.init_dims.width  / 2); 
        window_cfg.init_dims.y      = (sys_info->monitor.primary.pixel_height / 2) - (window_cfg.init_dims.height / 2); 
        pfm_window_open(&window_cfg);
    }

    IFB_INLINE void
    eng_context_startup_file_mngr(
        const eng_mem_map* mem_map) {

        const u32 file_granularity = size_kilobytes(64);
        file_mngr_startup(
            mem_map->files.size,
            file_granularity,
            mem_map->files.ptr
        );
    }

    IFB_INLINE void
    eng_context_startup_entity_mngr(
        const eng_mem_map* mem_map) {

        memory entity_mem;
        entity_mem.size = mem_map->entities.size;
        entity_mem.ptr  = mem_map->entities.ptr;
        entity_mngr_startup(entity_mem);
    }

    IFB_INLINE void
    eng_context_startup_memory_mngr(
        const eng_mem_map* mem_map) {

        memory arena_mem;
        arena_mem.size = mem_map->arenas.size;
        arena_mem.ptr  = mem_map->arenas.ptr;
        memory_mngr_startup(arena_mem);
    }

    IFB_INLINE void
    eng_context_startup_renderer(
        const eng_mem_map* mem_map) {

        // initialize the renderer
        memory mem_rndr;
        mem_rndr.ptr  = mem_map->rendering.ptr;
        mem_rndr.size = mem_map->rendering.size;
        renderer_context_startup        (mem_rndr);

        // open shader files
        const file_handle file_hnd_quad_vert    = file_ro_open_existing ("quad-shader-vertex.glsl");
        const file_handle file_hnd_quad_frag    = file_ro_open_existing ("quad-shader-fragment.glsl");
        const file_handle file_hnd_dir_giz_vert = file_ro_open_existing ("direction-gizmo-shader-vert.glsl");
        const file_handle file_hnd_dir_giz_frag = file_ro_open_existing ("direction-gizmo-shader-frag.glsl");

        // read quad shaders        
        shader_source file_src_quad_vert;
        shader_source file_src_quad_frag;
        file_src_quad_vert.size = file_get_size (file_hnd_quad_vert); 
        file_src_quad_vert.data = file_read     (file_hnd_quad_vert, file_src_quad_vert.size);
        file_src_quad_frag.size = file_get_size (file_hnd_quad_frag);
        file_src_quad_frag.data = file_read     (file_hnd_quad_frag, file_src_quad_frag.size); 
        
        // read direction gizmo shaders
        shader_source file_src_dir_giz_vert;
        shader_source file_src_dir_giz_frag;
        file_src_dir_giz_vert.size = file_get_size (file_hnd_dir_giz_vert); 
        file_src_dir_giz_vert.data = file_read     (file_hnd_dir_giz_vert, file_src_dir_giz_vert.size);
        file_src_dir_giz_frag.size = file_get_size (file_hnd_dir_giz_frag);
        file_src_dir_giz_frag.data = file_read     (file_hnd_dir_giz_frag, file_src_dir_giz_frag.size); 

        // initialize shaders
        renderer_quad_shader_init            (file_src_quad_vert,    file_src_quad_frag);
        renderer_hello_quad_shader_init      (file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (file_src_dir_giz_vert, file_src_dir_giz_frag);

        // close the shader files
        file_close(file_hnd_quad_vert);
        file_close(file_hnd_quad_frag);
        file_close(file_hnd_dir_giz_vert);
        file_close(file_hnd_dir_giz_frag);
    }

    IFB_INLINE void
    eng_context_startup_component_mngr(
        const eng_mem_map* mem_map) {

        memory mem;
        mem.ptr  = mem_map->components.ptr;
        mem.size = mem_map->components.size;
        component_mngr_startup(mem);
    }

};