#pragma once

#include "ifb-gui.hpp"
#include "stdio.h"
#include "string.h"

namespace ifb {


    struct control_label {
        char cstr[32];
    };

    IFB_INTERNAL const control_label
    gui_control_create_label(
        const char* cstr) {

        static u32 control_id = 0;

        // init new label
        control_label lbl;
        memset(lbl.cstr, 0, sizeof(lbl));

        // copy the string to the label
        const u32 size_ctrl_lbl = sizeof(control_label);
        u32 cstr_len = strnlen_s(cstr, size_ctrl_lbl);
        strncpy_s(lbl.cstr, cstr, cstr_len);

        // copy the number to the label
        lbl.cstr[cstr_len++] = '#';
        lbl.cstr[cstr_len++] = '#';
        const u32 size_remaining = (size_ctrl_lbl - cstr_len);
        sprintf_s(&lbl.cstr[cstr_len++],size_remaining,"%d",control_id++);

        // terminate the label and return
        lbl.cstr[31] = 0;
        return(lbl);
    }

    IFB_INTERNAL void
    gui_control_vec3(
        const char* name,
        vec3&       v) {

        static control_label lbl_x = gui_control_create_label("X");
        static control_label lbl_y = gui_control_create_label("Y");
        static control_label lbl_z = gui_control_create_label("Z");

        ImGui::SeparatorText(name);
        ImGui::DragFloat(lbl_x.cstr, &v.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
        ImGui::DragFloat(lbl_y.cstr, &v.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
        ImGui::DragFloat(lbl_z.cstr, &v.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
    }

    // a column as wide as its header or the widest value it shows, whichever is wider, plus extra_width for padding like a button's
    // without a width, a resizable table sizes a fixed column from what it held the first frame it was drawn,
    // which can be nothing, and that width is saved in imgui.ini
    IFB_INTERNAL void
    gui_table_column_fixed(
        const char* label,
        const char* widest_value,
        const f32   extra_width) {

        const f32 width_label = ImGui::CalcTextSize(label, NULL, true).x;
        const f32 width_value = ImGui::CalcTextSize(widest_value).x + extra_width;
        const f32 width       = (width_label > width_value) ? width_label : width_value;

        ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthFixed, width);
    }

    // a column that shares whatever the fixed columns leave, by weight
    IFB_INTERNAL void
    gui_table_column_stretch(
        const char* label,
        const f32   weight) {

        ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthStretch, weight);
    }
};

