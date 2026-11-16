#pragma once

#include "ifb-gui.hpp"
#include "gui-engine.cpp"
#include "gui-engine-profiler.cpp"
#include "gui-entity-manager.cpp"
#include "gui-renderer-camera.cpp"
#include "gui-controls.cpp"
#include "gui-quad-manager.cpp"            
#include "gui-game-player-rig.cpp"
#include "imgui.h"
#include "gui-engine-particle-system.cpp"
#include "gui-game-asset-dictionary.cpp"
#include "gui-game-asset-database.cpp"
#include "gui-engine-sprite-atlas-builder.cpp"
#include "gui-engine-sprite-atlas-inspector.cpp"
#include "gui-engine-audio-test.cpp"
#include "gui-engine-wav-player.cpp"
#include "gui-engine-memory.cpp"
#include "gui-engine-renderer.cpp"
#include "gui-engine-hash-calculator.cpp"
#include "gui-engine-map-builder.cpp"
#include "gui-game-audio.cpp"
#include "gui-engine-font-viewer.cpp"
#include "gui-game-tile-coords.cpp"
#include "gui-game-physics-test.cpp"

static bool _gui_is_open;

namespace ifb {

    void
    gui_open(
        void) {

        _gui_is_open = true;
    }

    IFB_INTERNAL void
    gui_close(
        void) {

        _gui_is_open = false;
    }

    game_scene_id_e
    gui_render(
        const game_context* game_ctx) {

        IFB_PROFILE_FUNCTION();

        if (!_gui_is_open) {
            return(game_scene_id_e_none);
        }

        return(gui_main_window(game_ctx));
    }

    IFB_INTERNAL game_scene_id_e
    gui_main_window(
        const game_context* game_ctx) {

        // the game > scenes labels, indexed by game_scene_id_e
        // none doesn't have one, and the test scenes are under game > testing instead
        static const char* const scene_labels[game_scene_id_e_count] = {
            NULL,
            _gui_str->menu_game_menu_scenes_item_dev,
            _gui_str->menu_game_menu_scenes_item_cliff_town,
            NULL,
        };
        static_assert(game_scene_id_e_count == 4, "add the new scene to scene_labels");

        game_scene_id_e scene_picked = game_scene_id_e_none;

        static bool is_selected            = false;
        static bool eng_imgui_demo         = false;
        static bool eng_implot_demo        = false;
        static bool eng_system             = false;
        static bool eng_particle_system    = false; 
        static bool rndr_camera            = false;
        static bool rndr_stats             = false;
        static bool entity_manager         = false;
        static bool quad_manager           = false;
        static bool player_rig             = false;
        static bool game_audio             = false;
        static bool asset_dictionary       = false;
        static bool asset_database         = false;
        static bool sprite_atlas_builder   = false;
        static bool sprite_atlas_inspector = false;
        static bool eng_audio_test         = false;
        static bool eng_wav_player         = false;
        static bool eng_memory             = false;
        static bool eng_hash_calculator    = false;
        static bool eng_map_builder        = false;
        static bool eng_font_viewer        = false;
        static bool game_tile_coords       = false;

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu(_gui_str->menu_engine)) {

                ImGui::MenuItem (_gui_str->menu_engine_item_imgui_demo,           NULL, &eng_imgui_demo);
                ImGui::MenuItem (_gui_str->menu_engine_item_implot_demo,          NULL, &eng_implot_demo);
                ImGui::MenuItem (_gui_str->menu_engine_item_system,               NULL, &eng_system);
                ImGui::MenuItem (_gui_str->menu_engine_item_particle_system,      NULL, &eng_particle_system);
                ImGui::MenuItem (_gui_str->menu_engine_item_audio_test,           NULL, &eng_audio_test);
                ImGui::MenuItem (_gui_str->menu_engine_item_wav_player,           NULL, &eng_wav_player);
                ImGui::MenuItem (_gui_str->menu_engine_item_memory,               NULL, &eng_memory);

                if (ImGui::BeginMenu(_gui_str->menu_engine_menu_renderer)) {
                    ImGui::MenuItem (_gui_str->menu_engine_menu_renderer_item_stats,  NULL, &rndr_stats);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_renderer_item_camera, NULL, &rndr_camera);

                    // the renderer owns the grid state, so read it every time the menu is open
                    bool rndr_grid = eng_renderer_is_grid_visible();
                    if (ImGui::MenuItem(_gui_str->menu_engine_menu_renderer_item_grid, NULL, &rndr_grid)) {
                        eng_renderer_set_grid_visible(rndr_grid);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(_gui_str->menu_engine_menu_sprite_tools)) {
                    ImGui::MenuItem (_gui_str->menu_engine_menu_sprite_tools_item_atlas_builder,   NULL, &sprite_atlas_builder);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_sprite_tools_item_atlas_inspector, NULL, &sprite_atlas_inspector);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(_gui_str->menu_engine_menu_tools)) {
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_item_hash_calculator, NULL, &eng_hash_calculator);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_item_map_builder,     NULL, &eng_map_builder);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_font_viewer,          NULL, &eng_font_viewer);

                    // the viewer is its own program, checked while it's open
                    const bool profiler_available = gui_engine_profiler_is_available();
                    const bool profiler_running   = gui_engine_profiler_is_running();
                    if (ImGui::MenuItem(_gui_str->menu_engine_menu_tools_item_profiler, NULL, profiler_running, profiler_available && !profiler_running)) {
                        gui_engine_profiler_open();
                    }
                    if (!profiler_available) {
                        ImGui::SetItemTooltip("%s", _gui_str->menu_engine_menu_tools_item_profiler_unavailable);
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_gui_str->menu_game)) {

                ImGui::MenuItem (_gui_str->menu_game_item_player_rig, NULL, &player_rig);
                ImGui::MenuItem (_gui_str->menu_game_item_audio,      NULL, &game_audio);
                ImGui::MenuItem (_gui_str->menu_game_item_tile_coords, NULL, &game_tile_coords);

                // the active scene is checked, and they're all disabled while a switch is underway
                if (ImGui::BeginMenu(_gui_str->menu_game_menu_scenes, game_ctx != NULL)) {

                    const game_scene_state& scene        = game_ctx->scene;
                    const bool              is_switching = (
                        scene.phase == game_scene_phase_e_switching ||
                        scene.next  != game_scene_id_e_none
                    );

                    for (
                        u32 i_scene = game_scene_id_e_none + 1;
                            i_scene < game_scene_id_e_count;
                          ++i_scene) {

                        if (scene_labels[i_scene] == NULL) continue;

                        const bool is_active = (scene.active.val == i_scene);
                        if (ImGui::MenuItem(scene_labels[i_scene], NULL, is_active, !is_switching)) {
                            scene_picked = (game_scene_id_e)i_scene;
                        }
                    }
                    ImGui::EndMenu();
                }

                // the test scenes, picked like the scenes are
                if (ImGui::BeginMenu(_gui_str->menu_game_menu_testing, game_ctx != NULL)) {

                    const game_scene_state& scene        = game_ctx->scene;
                    const bool              is_switching = (
                        scene.phase == game_scene_phase_e_switching ||
                        scene.next  != game_scene_id_e_none
                    );
                    const bool is_active = (scene.active.val == game_scene_id_e_physics);

                    if (ImGui::MenuItem(_gui_str->menu_game_menu_testing_item_physics, NULL, is_active, !is_switching)) {
                        scene_picked = game_scene_id_e_physics;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(_gui_str->menu_game_menu_assets)) {
                    
                    ImGui::MenuItem(_gui_str->menu_game_menu_assets_item_dictionary, NULL, &asset_dictionary);
                    ImGui::MenuItem(_gui_str->menu_game_menu_assets_item_database,   NULL, &asset_database);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_gui_str->menu_entities)) {

                ImGui::MenuItem (_gui_str->menu_entity_item_manager,   NULL, &entity_manager);
                ImGui::MenuItem (_gui_str->menu_entity_item_quad_mngr, NULL, &quad_manager);
                ImGui::EndMenu  ();
            }

            ImGui::EndMainMenuBar();
        }

        gui_engine_imgui_demo             (eng_imgui_demo);
        gui_engine_implot_demo            (eng_implot_demo);
        gui_engine_particle_system        (eng_particle_system);
        gui_engine_sprite_atlas_builder   (sprite_atlas_builder);
        gui_engine_sprite_atlas_inspector (sprite_atlas_inspector);
        gui_engine_audio_test             (eng_audio_test);
        gui_engine_wav_player             (eng_wav_player);
        gui_engine_memory                 (eng_memory);
        gui_engine_hash_calculator        (eng_hash_calculator);
        gui_engine_map_builder            (eng_map_builder);
        gui_renderer_camera               (rndr_camera);
        gui_engine_renderer               (rndr_stats);
        gui_entity_manager                (entity_manager);
        gui_quad_manager                  (quad_manager);
        gui_game_player_rig               (player_rig, game_ctx);
        gui_game_audio                    (game_audio, game_ctx);
        gui_game_asset_dictionary         (asset_dictionary);
        gui_game_asset_database           (asset_database);
        gui_engine_font_viewer            (eng_font_viewer);
        gui_game_tile_coords              (game_tile_coords, game_ctx);
        gui_game_physics_test             (game_ctx);

        return(scene_picked);
    }
};
