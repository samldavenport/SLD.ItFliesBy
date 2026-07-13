#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_quad_manager(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_entity_item_quad_mngr, &is_open)) {

        }

    ImGui::End();
    }
};