#pragma once

#include "gui.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    gui_engine_imgui_demo(
        void) {

        gui_validate();

        const gui_strings* str = _gui->str;
        
        bool is_open = _gui->window_flags.get_engine_imgui_demo();

        if (!is_open) return;

        ImGui::ShowDemoWindow(&is_open);

        _gui->window_flags.update_engine_imgui_demo(is_open);
    }

    IFB_INTERNAL void
    gui_engine_system(
        void) {

        gui_validate();

    }
}