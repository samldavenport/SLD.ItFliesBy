#pragma once

#include "gui.hpp"
#include "eng-internal.hpp"
#include "stdio.h"
namespace ifb {

    IFB_INTERNAL void
    gui_renderer_camera(
        gui* g) {

        assert(g);

        static gui_strings* str = g->str;
        static vec3 camera_target;
        static vec3 camera_origin;
        static view camera_view_matrix;

        bool is_open = g->window_flags.get_renderer_camera();
        if (!is_open) return;


        if (ImGui::Begin(str->window_renderer_camera, &is_open)) {

            ImGui::SeparatorText("Origin");
            renderer_camera_get_origin(_eng_context->renderer, camera_origin);
            ImGui::DragFloat("X##cam-org", &camera_origin.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Y##cam-org", &camera_origin.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Z##cam-org", &camera_origin.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            renderer_camera_set_origin(_eng_context->renderer, camera_origin);

            ImGui::SeparatorText("Target");
            renderer_camera_get_target(_eng_context->renderer, camera_target);
            ImGui::DragFloat("X##cam-tgt", &camera_target.x, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Y##cam-tgt", &camera_target.y, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            ImGui::DragFloat("Z##cam-tgt", &camera_target.z, GUI_DRAG_FLOAT_SPEED, GUI_DRAG_FLOAT_MIN, GUI_DRAG_FLOAT_MAX, "%.5f");
            renderer_camera_set_target(_eng_context->renderer, camera_target);

            const ImGuiTableFlags tbl_flags = (
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg 
            );


            ImGui::SeparatorText("View Matrix");
            renderer_camera_get_view(_eng_context->renderer, camera_view_matrix);
            if (ImGui::BeginTable("tbl-cam-view-matrix",4,tbl_flags)) {

                for (
                    u32 row = 0; row < 4; ++row
                ) {
                    ImGui::TableNextRow();

                    for (
                        u32 col = 0; col < 4; ++col
                    ) {

                        char txt[16];
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

        g->window_flags.update_renderer_camera(is_open);
    }
};

