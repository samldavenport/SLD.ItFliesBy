#pragma once

#include "ifb-gui.hpp"

namespace ifb {
  
    IFB_INTERNAL void
    gui_engine_map_builder(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_engine_menu_tools_item_map_builder, &is_open)) {

        }
        ImGui::End();
    }
};
