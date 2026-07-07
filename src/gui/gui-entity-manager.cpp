#pragma once

#include "gui.hpp"
#include "entity.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 SELECTION_LABEL_SIZE = 16;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static u32    _selected_row                          = INVALID_INDEX;
    static cchar  _input_tag[ENTITY_TAG_SIZE]            = {0};
    static cchar  _selection_label[SELECTION_LABEL_SIZE] = {0};
    static entity _selected_entity                       = {0};

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void entity_input   (void);
    IFB_INLINE void entity_table   (void);
    IFB_INLINE void entity_control (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_entity_manager(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin("Entity Manager", &is_open)) {

            entity_input();
            entity_table();
            entity_control();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    entity_input(
        void) {

        ImGui::InputText("Entity Tag", _input_tag, ENTITY_TAG_SIZE, ImGuiInputFlags_None);
        ImGui::SameLine();
        if (ImGui::Button("Create Entity")) {
            const entity_id id_new  = entity_create(_input_tag);
            assert(id_new != ENTITY_ID_INVALID);
            memset(_input_tag, 0, ENTITY_TAG_SIZE);
        }
    }

    IFB_INLINE void
    entity_table(
        void) {

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

                // get the entity and start the next row
                entity_lookup_by_index_dense(e, index);
                ImGui::TableNextRow();

                //---------------------------
                // DENSE INDEX
                //---------------------------

                ImGui::TableSetColumnIndex(tbl_clmn_indx_dense_index);
                memset    (_selection_label, 0, SELECTION_LABEL_SIZE);
                sprintf_s (_selection_label, SELECTION_LABEL_SIZE, "%d", e.index_dense);
                const bool is_selected     = (_selected_row == index);
                const bool render_selected = ImGui::Selectable(_selection_label, is_selected, ImGuiSelectableFlags_SpanAllColumns);
                if (render_selected) {
                    _selected_row    = index;
                    _selected_entity = e;
                }

                //---------------------------
                // ENTITY ID
                //---------------------------

                ImGui::TableSetColumnIndex(tbl_clmn_indx_id);
                ImGui::Text("%08X", e.id);

                //---------------------------
                // TAG CSTR
                //---------------------------

                ImGui::TableSetColumnIndex(tbl_clmn_indx_tag);
                ImGui::Text("%s", e.tag);

                //---------------------------
                // ARCHETYPE
                //---------------------------

                ImGui::TableSetColumnIndex(tbl_clmn_indx_archetype);
                ImGui::Text("%08X", e.archetype);

                //---------------------------
                // SPARSE INDEX
                //---------------------------

                ImGui::TableSetColumnIndex(tbl_clmn_indx_sparse_index);
                ImGui::Text("%d", e.index_sparse);
            }

            ImGui::EndTable();
        }        
    }

    IFB_INLINE void
    entity_control(
        void) {

        // return if we don't have an entity selected
        if (_selected_row == INVALID_INDEX) {
            return;
        }

        // sanity check, the selected entity dense index
        // should match the selected row
        assert(_selected_entity.index_dense == _selected_row);

        const bool begin_table = ImGui::BeginTable(
            "tbl-entity-ctrl",
            2,
            ImGuiTableFlags_Borders
        ); 

        if (begin_table) {

            ImGui::TableSetupColumn("Property");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            const u32 tbl_clmn_index_property = 0;
            const u32 tbl_clmn_index_value    = 1;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(tbl_clmn_index_property);
            ImGui::Text("Tag:");
            ImGui::TableSetColumnIndex(tbl_clmn_index_value);
            ImGui::Text(_selected_entity.tag);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(tbl_clmn_index_property);
            ImGui::Text("ID:");
            ImGui::TableSetColumnIndex(tbl_clmn_index_value);
            ImGui::Text("%08X", _selected_entity.id);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(tbl_clmn_index_property);
            ImGui::Text("Archetype:");
            ImGui::TableSetColumnIndex(tbl_clmn_index_value);
            ImGui::Text("%08X", _selected_entity.archetype);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(tbl_clmn_index_property);
            ImGui::Text("Index Sparse:");
            ImGui::TableSetColumnIndex(tbl_clmn_index_value);
            ImGui::Text("%d", _selected_entity.index_sparse);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(tbl_clmn_index_property);
            ImGui::Text("Index Dense:");
            ImGui::TableSetColumnIndex(tbl_clmn_index_value);
            ImGui::Text("%d", _selected_entity.index_dense);


            ImGui::EndTable();
        }

        if (ImGui::Button("Destroy Entity")) {
            _selected_row = INVALID_INDEX;
            entity_destroy(_selected_entity.tag);
        }
    }
};