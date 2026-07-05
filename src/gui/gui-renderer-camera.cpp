#pragma once

#include "gui.hpp"
#include "eng-internal.hpp"
#include "stdio.h"
namespace ifb {

    IFB_INTERNAL void
    gui_renderer_camera(
        bool& is_open) {

        if (!is_open) return;

        static vec3         camera_target;
        static vec3         camera_origin;
        static vec3         camera_forward;
        static vec3         camera_right;
        static vec3         camera_up;
        static mat4         camera_view_matrix;

        renderer_camera_get_target (camera_target);
        renderer_camera_get_origin (camera_origin);
        renderer_camera_get_view   (camera_view_matrix);

        camera_forward = xform_view_forward (camera_view_matrix);
        camera_right   = xform_view_right   (camera_view_matrix);
        camera_up      = xform_view_up      (camera_view_matrix);

        if (ImGui::Begin(_gui_str->window_renderer_camera, &is_open)) {

            ImGui::SeparatorText("Camera Vectors");

            if (ImGui::BeginTable("tbl-camera-vectors",4,ImGuiTableFlags_Borders)) {
                
                //---------------------------
                // COLUMN HEADER ROW
                //---------------------------

                ImGui::TableSetupColumn("Vector");
                ImGui::TableSetupColumn("X");
                ImGui::TableSetupColumn("Y");
                ImGui::TableSetupColumn("Z");
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

                //---------------------------
                // Vector column header
                //---------------------------
                
                ImGui::TableNextColumn();
                ImGui::TableHeader("Vector");

                //---------------------------
                // X column header
                //---------------------------
                
                ImGui::TableNextColumn ();
                ImGui::TableSetBgColor (ImGuiTableBgTarget_CellBg, IM_COL32(0xCC,0x24,0x1d,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Header,           IM_COL32(0xCC,0x24,0x1d,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_HeaderHovered,    IM_COL32(0xCC,0x24,0x1d,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Text,             IM_COL32(0x28,0x28,0x28,0xFF));
                ImGui::TableHeader     ("X");
                ImGui::PopStyleColor   (3);

                //---------------------------
                // Y column header
                //---------------------------
                
                ImGui::TableNextColumn ();
                ImGui::TableSetBgColor (ImGuiTableBgTarget_CellBg, IM_COL32(0x98,0x97,0x1A,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Header,           IM_COL32(0x98,0x97,0x1A,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_HeaderHovered,    IM_COL32(0x98,0x97,0x1A,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Text,             IM_COL32(0x28,0x28,0x28,0xFF));
                ImGui::TableHeader     ("Y");
                ImGui::PopStyleColor   (3);

                //---------------------------
                // Z column header
                //---------------------------

                ImGui::TableNextColumn ();
                ImGui::TableSetBgColor (ImGuiTableBgTarget_CellBg, IM_COL32(0x45,0x85,0x88,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Header,           IM_COL32(0x45,0x85,0x88,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_HeaderHovered,    IM_COL32(0x45,0x85,0x88,0xFF));
                ImGui::PushStyleColor  (ImGuiCol_Text,             IM_COL32(0x28,0x28,0x28,0xFF));
                ImGui::TableHeader     ("Z");
                ImGui::PopStyleColor   (3);

                //---------------------------
                // Origin vector
                //---------------------------

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Origin");
                ImGui::TableSetColumnIndex(1);
                ImGui::DragFloat("##cam-org-x", &camera_origin.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
                ImGui::TableSetColumnIndex(2);
                ImGui::DragFloat("##cam-org-y", &camera_origin.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
                ImGui::TableSetColumnIndex(3);
                ImGui::DragFloat("##cam-org-z", &camera_origin.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
                
                //---------------------------
                // Target vector
                //---------------------------

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Target");
                ImGui::TableSetColumnIndex(1);
                ImGui::DragFloat("##cam-tgt-x", &camera_target.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
                ImGui::TableSetColumnIndex(2);
                ImGui::DragFloat("##cam-tgt-y", &camera_target.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
                ImGui::TableSetColumnIndex(3);
                ImGui::DragFloat("##cam-tgt-z", &camera_target.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");

                //---------------------------
                // Forward vector
                //---------------------------

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Forward");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f",camera_forward.x);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%f",camera_forward.y);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%f",camera_forward.z);

                //---------------------------
                // right vector
                //---------------------------

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Right");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f",camera_right.x);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%f",camera_right.y);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%f",camera_right.z);

                //---------------------------
                // up vector
                //---------------------------

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Up");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f",camera_up.x);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%f",camera_up.y);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%f",camera_up.z);


                ImGui::EndTable();
            }

            const ImGuiTableFlags tbl_flags = (
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg 
            );

            ImGui::SeparatorText("View Matrix");
            if (ImGui::BeginTable("tbl-cam-view-matrix",4,tbl_flags)) {

                for (
                    u32 row = 0; row < 4; ++row
                ) {
                    ImGui::TableNextRow();

                    for (
                        u32 col = 0; col < 4; ++col
                    ) {

                        char txt[16];
                        memset(txt,0,sizeof(txt));
                        ImGui::TableSetColumnIndex(col);
                        const f32 val = camera_view_matrix.m[row + col];
                        sprintf_s(txt,sizeof(txt),"%f",val);
                        ImGui::TextUnformatted(txt);
                    }
                }

                ImGui::EndTable();
            }

            ImGui::End();
        }

        renderer_camera_set_target(camera_target);
        renderer_camera_set_origin(camera_origin);
    }
};

