#pragma once

#include "gui.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_renderer_camera(
        gui* g) {

        assert(g);

        static gui_strings* str = g->str;

        bool is_open = g->window_flags.get_renderer_camera();
        if (!is_open) return;

        static vec3 camera_target;
        static vec3 camera_origin;

        if (ImGui::Begin(str->window_renderer_camera, &is_open)) {

            renderer_camera_get_origin (_eng_context->renderer, &camera_origin);
            renderer_camera_get_target (_eng_context->renderer, &camera_target);

            gui_control_vec3 (str->camera_origin, &camera_origin);
            gui_control_vec3 (str->camera_target, &camera_target);

            renderer_camera_set_origin (_eng_context->renderer, &camera_origin);
            renderer_camera_set_target (_eng_context->renderer, &camera_target);

            ImGui::End();
        }

        g->window_flags.update_renderer_camera(is_open);
    }
};

