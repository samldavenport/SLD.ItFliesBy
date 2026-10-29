#pragma once

#include "entity.cpp"
#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "platform-internal.hpp"
#include "ifb.hpp"
#include "eng-context.hpp"
#include "imgui.h"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "particle-update.cpp"
#include "physics-manager.cpp"
#include "random-table.cpp"
#include "renderer.cpp"
#include "tiled-internal.hpp"
#include "eng-system.cpp"
#include "asset-internal.hpp"
#include "sprite-internal.hpp"
#include "ifb-platform.hpp"
#include "entity-internal.hpp"
#include "file-internal.hpp"
#include "map-internal.hpp"
#include "memory-internal.hpp"
#include "physics-internal.hpp"
#include "random-internal.hpp"
#include "renderer-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map,
        fptr_eng_game      game_callback,
        fptr_eng_render    render_callback) {
	
        const auto& config       = config_instance();

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
        _eng_context                  = eng_ctx;
        _eng_context->mem_map         = mem_map;
        _eng_context->game_callback   = game_callback;
        _eng_context->render_callback = render_callback;
        _eng_context->game_ctx        = game_ctx;
        _eng_context->system          = sys_info;  
        _eng_context->keyboard        = keyboard_input_create(); 
        _eng_context->renderer        = renderer_create(); 
        _eng_context->file_mngr       = file_mngr_create(); 
        _eng_context->entity_mngr     = entity_mngr_create(); 
        _eng_context->memory_mngr     = memory_mngr_create(); 
        _eng_context->cmpnt_mngr      = cmpnt_mngr_create();  
        _eng_context->phys_mngr       = phys_mngr_create();
        _eng_context->map_mngr        = map_mngr_create();
        _eng_context->particle_mngr   = particle_mngr_create();
        _eng_context->random_table    = random_table_create();
        _eng_context->asset_mngr      = asset_mngr_create();
        _eng_context->sprite_mngr     = sprite_mngr_create();
        _eng_context->mem_map         = mem_map;

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
            _eng_context->phys_mngr     != NULL &&
            _eng_context->particle_mngr != NULL &&
            _eng_context->random_table  != NULL &&
            _eng_context->asset_mngr    != NULL &&
            _eng_context->sprite_mngr   != NULL
        );

        return(_eng_context);
    }

    IFB_ENGINE_API void
    eng_context_startup(
        void) {

        const auto&        config   = config_instance();
        const eng_mem_map* mem_map  = _eng_context->mem_map;
        eng_system_info*   system   = _eng_context->system;
        renderer*          renderer = _eng_context->renderer;
   
        // start the platform context
        pfm_context_startup();

        _eng_context->seconds_per_frame = (1.0f / (f32)config.default_fps);
        _eng_context->frame_count       = 0;

        // refresh system info
        system_refresh_info();

        // open the window
        pfm_window_config window_cfg;
        window_cfg.title            = (char*)&config.window_title[0];
        window_cfg.init_dims.width  = config.window_start_width;
        window_cfg.init_dims.height = config.window_start_height;
        window_cfg.init_dims.x      = (system_get_primary_monitor_width()  / 2) - (window_cfg.init_dims.width  / 2); 
        window_cfg.init_dims.y      = (system_get_primary_monitor_height() / 2) - (window_cfg.init_dims.height / 2); 
        pfm_window_open(&window_cfg);
        
        // create memory reservations
        reservation* res_files      = reservation_create(mem_map->files);
        reservation* res_entities   = reservation_create(mem_map->entities);
        reservation* res_arenas     = reservation_create(mem_map->arenas);
        reservation* res_arenas_lg  = reservation_create(mem_map->arenas_large);
        reservation* res_renderer   = reservation_create(mem_map->rendering);
        reservation* res_components = reservation_create(mem_map->components);
        reservation* res_physics    = reservation_create(mem_map->physics);
        reservation* res_tiles      = reservation_create(mem_map->tiles);
        reservation* res_particles  = reservation_create(mem_map->particles);
        reservation* res_assets     = reservation_create(mem_map->assets);
        reservation* res_sprites    = reservation_create(mem_map->sprites);
        assert(res_files);
        assert(res_entities);
        assert(res_arenas);
        assert(res_arenas_lg);
        assert(res_renderer);
        assert(res_components);
        assert(res_physics);
        assert(res_tiles);
        assert(res_particles);
        assert(res_assets);
        assert(res_sprites);
       
        // start systems
        file_mngr_startup        (res_files);
        entity_mngr_startup      (res_entities);
        memory_mngr_startup      (res_arenas, res_arenas_lg);
        cmpnt_mngr_startup       (res_components);
        phys_mngr_startup        (res_physics);
        map_mngr_startup         (res_tiles);
        particle_mngr_startup    (res_particles);
        asset_mngr_startup       (res_assets);
        sprite_mngr_startup      (res_sprites);
        renderer_startup         (res_renderer);
    }

    IFB_ENGINE_API bool 
    eng_context_run(void) {

        static f32 elapsed_time = 0.0f;
    
        system_refresh_info();

        // get delta time
        system_update_time();
        const f32 dt =  system_get_delta_time_s();

        elapsed_time += dt;
        if (elapsed_time >= _eng_context->seconds_per_frame) {
            elapsed_time = 0.0f;
        }

        //TODO(SAM): pass the opengl context to the platform
        // start new frame
        if (elapsed_time == 0.0f) {
            ++_eng_context->frame_count;
            asset_mngr_update        ();
            pfm_window_frame_start   ();
            pfm_window_process_events();
            random_table_update      ();
        }

        // game callback    
        _eng_context->game_callback(
            _eng_context->game_ctx
        );

        // simulate physics
        phys_mngr_simulate(dt);

        // update particles, the emitters on entities go where the physics put them
        particle_mngr_follow_entities();
        particle_mngr_update_emitters(dt * 1000.0f);
        
        if (elapsed_time == 0.0f) {
       
            // render graphics
            renderer_draw_buffers();

            _eng_context->render_callback();

            // render frame
            pfm_window_frame_render();
        }

        // check if quit received
        const bool quit = pfm_window_quit_received();
        return (quit ? false : true);
    }
    
    IFB_ENGINE_API void
    eng_context_shutdown(
        void) {

        //TODO(SLD)

        asset_mngr_shutdown();

        // shut down the platform context
        pfm_context_shutdown();
    }

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the number of frames started since startup, counted at the fixed frame rate
    // the game callback can run several times in one frame, and sees the same count each time
    IFB_ENG_INTERNAL u64
    eng_context_get_frame_count(
        void) {

        assert(_eng_context);
        return(_eng_context->frame_count);
    }

    IFB_ENGINE_API ImGuiContext*
    eng_context_get_imgui(
        void) {

        ImGuiContext* ctx = ImGui::GetCurrentContext();
        assert(ctx);
        return(ctx);
    }
};
