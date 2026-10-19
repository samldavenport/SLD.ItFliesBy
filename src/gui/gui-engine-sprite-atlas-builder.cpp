#pragma once

#include "ifb-gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_engine_sprite_atlas_builder(
        bool& is_open) {

        if (!is_open) return;

        const bool render_window = ImGui::Begin(
            _gui_str->menu_engine_item_sprite_atlas_builder,
            &is_open
        );
        
        if (render_window) {
        
        }

        ImGui::End();
    }
};

