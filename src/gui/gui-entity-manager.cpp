#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_entity_manager(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin("Entity Manager", &is_open)) {


        }

        ImGui::End();
    }
};