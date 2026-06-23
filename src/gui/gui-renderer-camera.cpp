#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_renderer_camera(
        gui* g) {

        assert(g);

        static gui_strings* str = g->str;

        bool is_open = g->window_flags.get_renderer_camera();
        if (!is_open) return;


        if (ImGui::Begin(str->window_renderer_camera, &is_open)) {

            ImGui::End();
        }

        g->window_flags.update_renderer_camera(is_open);
    }
};

