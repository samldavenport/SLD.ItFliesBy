#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_control_vec3(
        const char* name,
        vec3*       v) {

        ImGui::SeparatorText(name);
        ImGui::DragFloat("X", &v->x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX);
        ImGui::DragFloat("Y", &v->y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX);
        ImGui::DragFloat("Z", &v->z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX);
    }
};

