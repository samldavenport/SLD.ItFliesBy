#pragma once

#include "ifb-gui.hpp"
#include "gui-engine.cpp"
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
#include "gui-engine-hash-calculator.cpp"
#include "gui-engine-map-builder.cpp"
#include "gui-game-audio.cpp"
#include "gui-engine-font-viewer.cpp"
#include "gui-game-tile-coords.cpp"

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

    void
    gui_render(
        const game_context* game_ctx) {

        if (!_gui_is_open) {
            return;
        }

        gui_main_window       (game_ctx);
    }

    IFB_INTERNAL void
    gui_main_window(
        const game_context* game_ctx) {

        static bool is_selected            = false;
        static bool eng_imgui_demo         = false;
        static bool eng_implot_demo        = false;
        static bool eng_system             = false;
        static bool eng_particle_system    = false; 
        static bool rndr_camera            = false; 
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

                if (ImGui::BeginMenu(_gui_str->menu_engine_menu_sprite_tools)) {
                    ImGui::MenuItem (_gui_str->menu_engine_menu_sprite_tools_item_atlas_builder,   NULL, &sprite_atlas_builder);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_sprite_tools_item_atlas_inspector, NULL, &sprite_atlas_inspector);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(_gui_str->menu_engine_menu_tools)) {
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_item_hash_calculator, NULL, &eng_hash_calculator);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_item_map_builder,     NULL, &eng_map_builder);
                    ImGui::MenuItem (_gui_str->menu_engine_menu_tools_font_viewer,          NULL, &eng_font_viewer);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_gui_str->menu_game)) {

                ImGui::MenuItem (_gui_str->menu_game_item_player_rig, NULL, &player_rig);
                ImGui::MenuItem (_gui_str->menu_game_item_audio,      NULL, &game_audio);
                ImGui::MenuItem (_gui_str->menu_game_item_tile_coords, NULL, &game_tile_coords);
               
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

            if (ImGui::BeginMenu(_gui_str->menu_renderer)) {
                ImGui::MenuItem (_gui_str->menu_renderer_item_camera, NULL, &rndr_camera);

                // the renderer owns the grid state, so read it every time the menu is open
                bool rndr_grid = eng_renderer_is_grid_visible();
                if (ImGui::MenuItem(_gui_str->menu_renderer_item_grid, NULL, &rndr_grid)) {
                    eng_renderer_set_grid_visible(rndr_grid);
                }

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
        gui_entity_manager                (entity_manager);
        gui_quad_manager                  (quad_manager);
        gui_game_player_rig               (player_rig, game_ctx);
        gui_game_audio                    (game_audio, game_ctx);
        gui_game_asset_dictionary         (asset_dictionary);
        gui_game_asset_database           (asset_database);
        gui_engine_font_viewer            (eng_font_viewer);
        gui_game_tile_coords              (game_tile_coords, game_ctx);
    }
};
