#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_control_vec3(
        const char* name,
        vec3&       v) {

        // TODO(SAM): we need to concatenate the names of the controls
        // so they are unique every time
        ImGui::SeparatorText(name);
        ImGui::DragFloat("X", &v.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
        ImGui::DragFloat("Y", &v.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
        ImGui::DragFloat("Z", &v.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
    }
};

