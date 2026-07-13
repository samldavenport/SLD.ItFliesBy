#pragma once

#include "gui.hpp"
#include "quad.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void quad_table (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_quad_manager(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_entity_item_quad_mngr, &is_open)) {

            quad_table();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    quad_table(
        void) {

        static const u32         SELECTION_LABEL_SIZE = 16;
        static       u32         selected_row                          = INVALID_INDEX;
        static       cchar       selection_label[SELECTION_LABEL_SIZE] = {0};
        static       quad_entity selected_quad                         = {0};


        if (ImGui::BeginTable("tbl-quad-mngr",10,ImGuiTableFlags_Borders)) {

            //---------------------------
            // COLUMN HEADER ROW
            //---------------------------

            ImGui::TableSetupColumn("Dense Index");
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Tag");
            ImGui::TableSetupColumn("Sparse Index");
            ImGui::TableSetupColumn("Position X");
            ImGui::TableSetupColumn("Position y");
            ImGui::TableSetupColumn("Position z");
            ImGui::TableSetupColumn("Color");
            ImGui::TableSetupColumn("Width");
            ImGui::TableSetupColumn("Height");
            ImGui::TableHeadersRow();

            const u32 clmn_dense_index  = 0;
            const u32 clmn_id           = 1;
            const u32 clmn_tag          = 2;
            const u32 clmn_sparse_index = 3;
            const u32 clmn_pos_x        = 4;
            const u32 clmn_pos_y        = 5;
            const u32 clmn_pos_z        = 6;
            const u32 clmn_color        = 7;
            const u32 clmn_width        = 8;
            const u32 clmn_height       = 9;

            //---------------------------
            // DATA ROWS
            //---------------------------

            quad_entity q;

            for (
                u32 index = 0;
                    index < _quad_mngr->all.count;
                  ++index
            ) {

                const entity_id id       = _quad_mngr->all.array[index];
                const bool      did_find = quad_lookup_by_id(q, id);
                assert(did_find);

                //---------------------------
                // DENSE INDEX
                //---------------------------

                ImGui::TableSetColumnIndex(clmn_dense_index);
                ImGui::TableSetColumnIndex(clmn_dense_index);
                memset    (selection_label, 0, SELECTION_LABEL_SIZE);
                sprintf_s (selection_label, SELECTION_LABEL_SIZE, "%d", q.index_dense);
                const bool is_selected     = (_selected_row == index);
                const bool render_selected = ImGui::Selectable(_selection_label, is_selected, ImGuiSelectableFlags_SpanAllColumns);
                if (render_selected) {
                    selected_row  = index;
                    selected_quad = q;
                }

                //---------------------------
                // ID
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_id);
                
                //---------------------------
                // TAG
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_tag);
                
                //---------------------------
                // SPARSE INDEX
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_sparse_index);
                
                //---------------------------
                // POSITION X
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_x);
                
                //---------------------------
                // POSITION Y
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_y);
                
                //---------------------------
                // POSITION Z
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_z);
                
                //---------------------------
                // COLOR
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_color);
                
                //---------------------------
                // WIDTH
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_width);
                
                //---------------------------
                // HEIGHT
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_height);

            }



            ImGui::EndTable();
        }
    }
};