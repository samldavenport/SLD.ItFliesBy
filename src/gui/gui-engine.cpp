#pragma once

#include "gui.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    gui_engine_imgui_demo(
        bool& is_open) {

        if (!is_open) return;

        ImGui::ShowDemoWindow(&is_open);
    }

    IFB_INTERNAL void
    gui_engine_system(
        void) {

    }
}