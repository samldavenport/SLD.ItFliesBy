#pragma once

#include "gui.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    gui_engine_imgui_demo(
        gui* g) {

        gui_validate(g);

        const gui_strings* str = g->str;
        
        bool is_open = g->window_flags.get_engine_imgui_demo();

        if (!is_open) return;

        ImGui::ShowDemoWindow(&is_open);

        g->window_flags.update_engine_imgui_demo(is_open);
    }

    IFB_INTERNAL void
    gui_engine_system(
        gui* g) {

        gui_validate(g);

    }
}