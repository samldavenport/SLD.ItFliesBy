#pragma once

#include "gui.hpp"
#include "gui-engine.cpp"
#include "gui-entity-manager.cpp"
#include "gui-renderer-camera.cpp"
#include "gui-controls.cpp"

static bool _gui_is_open;

namespace ifb {

    IFB_INTERNAL void
    gui_open(
        void) {

        _gui_is_open = true;
    }

    IFB_INTERNAL void
    gui_close(
        void) {

        _gui_is_open = false;
    }

    IFB_INTERNAL void
    gui_render(
        void) {

        if (!_gui_is_open) {
            return;
        }

        gui_main_window       ();
    }

    IFB_INTERNAL void
    gui_main_window(
        void) {

        static bool is_selected    = false;
        static bool eng_imgui_demo = false;
        static bool eng_system     = false;
        static bool rndr_camera    = false; 
        static bool entity_manager = false;

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu(_gui_str->menu_engine)) {

                ImGui::MenuItem(_gui_str->menu_engine_item_imgui_demo, NULL, &eng_imgui_demo);
                ImGui::MenuItem(_gui_str->menu_engine_item_system,     NULL, &eng_system);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_gui_str->menu_entities)) {

                ImGui::MenuItem(_gui_str->menu_entity_item_manager, NULL, &entity_manager);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_gui_str->menu_renderer)) {

                ImGui::MenuItem(_gui_str->menu_renderer_item_camera, NULL, &rndr_camera);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        gui_engine_imgui_demo (eng_imgui_demo);
        gui_renderer_camera   (rndr_camera);
        gui_entity_manager    (entity_manager);
    }
};