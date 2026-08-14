#pragma once

#include "cartographer.cpp"
#include "entity.cpp"
#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-entity.hpp"
#include "ifb.hpp"
#include "eng-internal.hpp"
#include "physics-manager.cpp"
#include "physics.hpp"
#include "quad.cpp"
#include "renderer.cpp"
#include "renderer.hpp"

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
    IFB_INLINE void eng_context_startup_cmpnt_mngr      (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_quad_mngr       (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_phys_mngr       (const eng_mem_map* mem_map);
    IFB_INLINE void eng_context_startup_cartographer    (const eng_mem_map* mem_map);

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map,
        eng_game_proc      game_callback) {
	
        const auto& config = config_instance();


        // stack memory
        global_stack_create_and_init(mem_map);

        // allocate global memory
        auto eng_ctx     = global_alloc<eng_context>      (); 
        auto sys_info    = global_alloc<eng_system_info>  ();
        auto game_ctx    = global_alloc<eng_game_context> (); 
        assert(
            eng_ctx     != NULL &&
            sys_info    != NULL
        );

        // set context properties        
        _eng_context                = eng_ctx;
        _eng_context->mem_map       = mem_map;
        _eng_context->game_callback = game_callback;
        _eng_context->game_ctx      = game_ctx;
        _eng_context->system        = sys_info;  
        _eng_context->keyboard      = keyboard_input_create(); 
        _eng_context->renderer      = renderer_context_create(); 
        _eng_context->file_mngr     = file_mngr_create(); 
        _eng_context->entity_mngr   = entity_mngr_create(); 
        _eng_context->memory_mngr   = memory_mngr_create(); 
        _eng_context->cmpnt_mngr    = cmpnt_mngr_create();  
        _eng_context->quad_mngr     = quad_mngr_create();
        _eng_context->phys_mngr     = physics_mngr_create();
        _eng_context->crtgphr       = cartographer_create();
        _eng_context->mem_map       = mem_map;

        assert(
            _eng_context->mem_map       != NULL &&
            _eng_context->game_callback != NULL &&
            _eng_context->game_ctx      != NULL &&
            _eng_context->system        != NULL &&
            _eng_context->system        != NULL &&
            _eng_context->keyboard      != NULL &&
            _eng_context->renderer      != NULL &&
            _eng_context->file_mngr     != NULL &&
            _eng_context->entity_mngr   != NULL &&
            _eng_context->memory_mngr   != NULL &&
            _eng_context->cmpnt_mngr    != NULL &&
            _eng_context->quad_mngr     != NULL &&
            _eng_context->phys_mngr     != NULL &&
            _eng_context->mem_map       != NULL &&
            _eng_context->crtgphr       != NULL
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
    
        _eng_context->seconds_per_frame = (1.0f / (f32)config.default_fps);

        eng_context_startup_get_system_info (_eng_context->system);
        eng_context_startup_file_mngr       (mem_map);
        eng_context_startup_entity_mngr     (mem_map);
        eng_context_startup_memory_mngr     (mem_map);
        eng_context_startup_cmpnt_mngr      (mem_map);
        eng_context_startup_quad_mngr       (mem_map);
        eng_context_startup_phys_mngr       (mem_map);
        eng_context_startup_open_window     (config, system);
        eng_context_startup_renderer        (mem_map);
        eng_context_startup_cartographer    (mem_map);
    }

    IFB_ENGINE_API void
    eng_context_run(void) {

        static f32 elapsed_time = 0.0f;

        while(true) {

            // get delta time
            eng_system_update_time();
            const f32 dt =  eng_system_get_delta_time_s();

            elapsed_time += dt;
            if (elapsed_time >= _eng_context->seconds_per_frame) {
                elapsed_time = 0.0f;
            }

            //TODO(SAM): pass the opengl context to the platform
            // start new frame
            if (elapsed_time == 0.0f) {
                pfm_window_frame_start   ();
                pfm_window_process_events();
            }

            // game callback    
            _eng_context->game_callback(
                _eng_context->game_ctx
            );

            // simulate physics
            physics_mngr_simulate(dt);

            
            if (elapsed_time == 0.0f) {
           
                // render graphics
                renderer_context_draw_buffers();

                // render gui
                gui_render();

                // render frame
                pfm_window_frame_render();
            }

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

        const auto& cfg         = config_instance();
        const u32   init_width  = cfg.window_start_width;
        const u32   init_height = cfg.window_start_height;
        
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
        const file_handle file_hnd_grid_vert    = file_ro_open_existing ("grid-vert.glsl");
        const file_handle file_hnd_grid_frag    = file_ro_open_existing ("grid-frag.glsl");

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
        
        // initialize shaders
        renderer_quad_shader_init            (file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (file_src_dir_giz_vert, file_src_dir_giz_frag);
        renderer_grid_shader_init(file_src_grid_vert, file_src_grid_frag);

        // close the shader files
        file_close(file_hnd_quad_vert);
        file_close(file_hnd_quad_frag);
        file_close(file_hnd_dir_giz_vert);
        file_close(file_hnd_dir_giz_frag);
        file_close(file_hnd_grid_vert);
        file_close(file_hnd_grid_frag);
    }

    IFB_INLINE void
    eng_context_startup_cmpnt_mngr(
        const eng_mem_map* mem_map) {

        memory mem;
        mem.ptr  = mem_map->components.ptr;
        mem.size = mem_map->components.size;
        cmpnt_mngr_startup(mem);
    }

    IFB_INLINE void
    eng_context_startup_quad_mngr(
        const eng_mem_map* mem_map) {

        memory mem;
        mem.ptr  = mem_map->quads.ptr;
        mem.size = mem_map->quads.size;
        quad_mngr_startup(mem);
    }

    IFB_INLINE void
    eng_context_startup_phys_mngr(
        const eng_mem_map* mem_map) {

        memory mem;
        mem.ptr  = mem_map->physics.ptr;
        mem.size = mem_map->physics.size;
        physics_mngr_startup(mem);
    }

    IFB_INLINE void
    eng_context_startup_cartographer(
        const eng_mem_map* mem_map) {

        memory mem;
        mem.ptr  = mem_map->cartographer.ptr;
        mem.size = mem_map->cartographer.size;

        cartographer_startup(mem);
    }
};
