#pragma once

#include "ifb-gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_engine_sprite_atlas_inspector(
        bool& is_open) {

        if (!is_open) return;

        const bool render_window = ImGui::Begin(
            _gui_str->menu_engine_menu_sprite_tools_item_atlas_inspector,
            &is_open
        );

        if (render_window) {

        }

        ImGui::End();
    }
};
