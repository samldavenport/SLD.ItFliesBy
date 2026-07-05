#pragma once

#include "gui.hpp"
#include "entities.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_entity_manager(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin("Entity Manager", &is_open)) {

            if (ImGui::BeginTable("tbl-entity-mngr", 5, ImGuiTableFlags_Borders)) {

                //---------------------------
                // COLUMN HEADER ROW
                //---------------------------

                ImGui::TableSetupColumn("Dense Index");
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Tag");
                ImGui::TableSetupColumn("Archetype");
                ImGui::TableSetupColumn("Sparse Index");
                ImGui::TableHeadersRow();

                const u32 tbl_clmn_indx_dense_index  = 0;
                const u32 tbl_clmn_indx_id           = 1;
                const u32 tbl_clmn_indx_tag          = 2;
                const u32 tbl_clmn_indx_archetype    = 3;
                const u32 tbl_clmn_indx_sparse_index = 4;

                //---------------------------
                // DATA ROWS
                //---------------------------

                entity e;
                for (
                    u32 index = 0;
                        index < _entity_mngr->count;
                      ++index
                ) {
                    entity_lookup_by_index_dense(e, index);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(tbl_clmn_indx_dense_index);
                    ImGui::Text("%d", e.index_dense);

                    ImGui::TableSetColumnIndex(tbl_clmn_indx_id);
                    ImGui::Text("%0X", e.id);

                    ImGui::TableSetColumnIndex(tbl_clmn_indx_tag);
                    ImGui::Text("%s", e.tag);
                    
                    ImGui::TableSetColumnIndex(tbl_clmn_indx_archetype);
                    ImGui::Text("%0X", e.archetype);

                    ImGui::TableSetColumnIndex(tbl_clmn_indx_sparse_index);
                    ImGui::Text("%d", e.index_sparse);
                }
 
                ImGui::EndTable();
            }
        }

        ImGui::End();
    }
};