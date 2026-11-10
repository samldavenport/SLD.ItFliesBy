#pragma once

#include "ifb-gui.hpp"

namespace ifb {

    struct font_viewer_state {

    } static _font_viewer_state;

    IFB_INTERNAL void
    gui_engine_font_viewer(
        bool& is_open) {

        if (!is_open) return;

        const bool begin = ImGui::Begin(_gui_str->menu_engine_menu_tools_font_viewer, &is_open);
        if (begin) {

        }
        ImGui::End();
    }

};
