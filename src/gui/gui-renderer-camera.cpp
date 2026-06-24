#pragma once

#include "gui.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_renderer_camera(
        gui* g) {

        assert(g);

        static gui_strings* str = g->str;
        static vec3 camera_target;
        static vec3 camera_origin;
        
        bool is_open = g->window_flags.get_renderer_camera();
        if (!is_open) return;


        if (ImGui::Begin(str->window_renderer_camera, &is_open)) {
        
            static vec3 origin, target;

            renderer_camera_get_origin(_eng_context->renderer, origin);
            renderer_camera_get_target(_eng_context->renderer, target);

            ImGui::SeparatorText("Origin");
            ImGui::DragFloat("X##cam-org", &origin.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Y##cam-org", &origin.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Z##cam-org", &origin.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");

            ImGui::SeparatorText("Target");
            ImGui::DragFloat("X##cam-tgt", &target.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Y##cam-tgt", &target.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Z##cam-tgt", &target.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");

            renderer_camera_set_origin(_eng_context->renderer, origin);
            renderer_camera_set_target(_eng_context->renderer, target);

            ImGui::End();
        }

        g->window_flags.update_renderer_camera(is_open);
    }
};

