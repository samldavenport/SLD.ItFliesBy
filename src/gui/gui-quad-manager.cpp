#pragma once

#include "ifb-engine.hpp"
#include "ifb-entity.hpp"
#include "ifb-gui.hpp"
#include "ifb-entity.hpp"

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

            static arena_handle a;
            static entity_list* quad_list;
            if (a == NULL) {
                a = eng_arena_alloc();
                quad_list = entity_list_arena_create(a);
            }
            assert(a != NULL);
            entity_list_validate(quad_list);

            entity_query query;
            query.has_all.val = ENTITY_ARCHETYPE_QUAD.val;
            const bool has_entities = eng_entity_query(quad_list, query);    
            if (!has_entities) {
                return;
            }

            const u32 quad_list_count = entity_list_count(quad_list);
            for (
                u32 index = 0;
                    index < quad_list_count;  
                  ++index
            ) {

                const entity_id id       = entity_list_index(quad_list, index); 
                const bool      did_find = eng_entity_quad_lookup(q, id);
                assert(did_find);
                ImGui::TableNextRow();

                //---------------------------
                // DENSE INDEX
                //---------------------------

                ImGui::TableSetColumnIndex(clmn_dense_index);
                memset    (selection_label, 0, SELECTION_LABEL_SIZE);
                sprintf_s (selection_label, SELECTION_LABEL_SIZE, "%d", q.index_dense);
                const bool is_selected     = (selected_row == index);
                const bool render_selected = ImGui::Selectable(selection_label, is_selected, ImGuiSelectableFlags_SpanAllColumns);
                if (render_selected) {
                    selected_row  = index;
                    selected_quad = q;
                }

                //---------------------------
                // ID
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_id);
                ImGui::Text("%08X", q.id);

                //---------------------------
                // TAG
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_tag);
                ImGui::Text("%s", q.tag);

                //---------------------------
                // SPARSE INDEX
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_sparse_index);
                ImGui::Text("%d", q.index_sparse);

                //---------------------------
                // POSITION X
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_x);
                ImGui::Text("%f", q.pos.x);


                //---------------------------
                // POSITION Y
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_y);
                ImGui::Text("%f", q.pos.y);
                
                //---------------------------
                // POSITION Z
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_pos_z);
                ImGui::Text("%f", q.pos.z);
                
                //---------------------------
                // COLOR
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_color);
                ImGui::Text("%08X", q.color.hex);

                //---------------------------
                // WIDTH
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_width);
                ImGui::Text("%f", q.dims.width);


                //---------------------------
                // HEIGHT
                //---------------------------
                
                ImGui::TableSetColumnIndex(clmn_height);
                ImGui::Text("%f", q.dims.height);
            }

            ImGui::EndTable();
        }
    }
};
