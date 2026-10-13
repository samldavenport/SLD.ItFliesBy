#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb-types.hpp"
#include "imgui.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 ASSET_DICTIONARY_PATH_SIZE      = IFB_CONFIG_FILE_PATH_SIZE;
    static constexpr f32 ASSET_DICTIONARY_PREVIEW_HEIGHT = 300.0f;

    static const char* _asset_dictionary_filter_type = "*.json";
    static const char* _asset_dictionary_filter_desc = "JSON";

    static const char* _asset_dictionary_type_names[asset_type_e_count] = {
        "Text",
        "Image"
    };

    static const char* _asset_dictionary_list_names[asset_type_e_count] = {
        "Text Files",
        "Image Files"
    };

    // the platform only reads the first 16 characters of a filter
    static const char* _asset_dictionary_type_filter_types[asset_type_e_count] = {
        "*.*",
        "*.png;*.jpg"
    };

    static const char* _asset_dictionary_type_filter_descs[asset_type_e_count] = {
        "All Files",
        "Images"
    };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the dictionary, the window owns the values of the next file
    struct asset_dictionary_state {
        bool                     is_dirty;
        const char*              status;
        char                     file_path [ASSET_DICTIONARY_PATH_SIZE]; // empty until the dictionary is opened or saved
        s32                      type;
        char                     name      [ASSET_DICTIONARY_PATH_SIZE];
        char                     path      [ASSET_DICTIONARY_PATH_SIZE];
        bool                     is_selected;
        bool                     is_preview_loaded;
        u32                      selected_id;
        asset_dictionary_preview preview;
    } static _asset_dictionary;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void asset_dictionary_save           (const char* path);
    IFB_INLINE void asset_dictionary_save_as        (void);
    IFB_INLINE void asset_dictionary_name_from_path (void);
    IFB_INLINE void asset_dictionary_controls_file  (void);
    IFB_INLINE void asset_dictionary_controls_add   (void);
    IFB_INLINE void asset_dictionary_table          (const u32 type);
    IFB_INLINE void asset_dictionary_select         (const u32 id);
    IFB_INLINE void asset_dictionary_preview_file   (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_game_asset_dictionary(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_game_menu_assets_item_dictionary, &is_open)) {

            asset_dictionary_controls_file ();
            asset_dictionary_controls_add  ();

            for (
                u32 type = 0;
                    type < asset_type_e_count;
                  ++type) {

                asset_dictionary_table(type);
            }

            asset_dictionary_preview_file();
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    asset_dictionary_save(
        const char* path) {

        auto& state = _asset_dictionary;

        if (!eng_asset_dictionary_save(path)) {
            state.status = "Failed to save the dictionary";
            return;
        }

        // the path may already be the one we have
        if (path != state.file_path) {
            strncpy_s(state.file_path, path, ASSET_DICTIONARY_PATH_SIZE - 1);
        }
        state.is_dirty = false;
        state.status   = "Saved the dictionary";

        // the dictionary has a location now, so a preview that needed one can load
        if (state.is_selected && !state.is_preview_loaded) {
            asset_dictionary_select(state.selected_id);
        }
    }

    IFB_INLINE void
    asset_dictionary_save_as(
        void) {

        const char* dlg_path = eng_file_dialog_save(_asset_dictionary_filter_type, _asset_dictionary_filter_desc);
        if (!dlg_path) return;

        char path[ASSET_DICTIONARY_PATH_SIZE];
        strncpy_s(path, dlg_path, ASSET_DICTIONARY_PATH_SIZE - 1);

        // the dialog doesn't add an extension, so a file name without one gets it here
        const char* file_name = path;
        bool        has_ext   = false;
        for (
            const char* c = path;
            *c != 0;
            ++c) {

            if (*c == '\\' || *c == '/') {
                file_name = c + 1;
                has_ext   = false;
            }
            else if (*c == '.') {
                has_ext = true;
            }
        }
        if (!has_ext && file_name[0] != 0) {
            strncat_s(path, ".json", ASSET_DICTIONARY_PATH_SIZE - strlen(path) - 1);
        }

        asset_dictionary_save(path);
    }

    // the default name is the file name without the extension
    IFB_INLINE void
    asset_dictionary_name_from_path(
        void) {

        auto& state = _asset_dictionary;

        const char* file_name = state.path;
        const char* ext       = NULL;
        for (
            const char* c = state.path;
            *c != 0;
            ++c) {

            if (*c == '\\' || *c == '/') {
                file_name = c + 1;
                ext       = NULL;
            }
            else if (*c == '.') {
                ext = c;
            }
        }

        // a name like .gitignore is all extension, so it is used as is
        const size_t length = (ext != NULL && ext != file_name)
            ? (size_t)(ext - file_name)
            : strlen(file_name);

        strncpy_s(state.name, file_name, length);
    }

    // new, open, and save the dictionary
    IFB_INLINE void
    asset_dictionary_controls_file(
        void) {

        auto& state = _asset_dictionary;

        if (ImGui::Button("New")) {
            if (eng_asset_dictionary_new()) {
                state.file_path[0] = 0;
                state.is_dirty     = false;
                state.is_selected  = false;
                state.status       = "Created a new dictionary";
            }
            else {
                state.status = "Failed to create a new dictionary";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Open...")) {

            // a failed load leaves the current dictionary alone
            const char* path = eng_file_dialog_open(_asset_dictionary_filter_type, _asset_dictionary_filter_desc);
            if (path && eng_asset_dictionary_load(path)) {
                strncpy_s(state.file_path, path, ASSET_DICTIONARY_PATH_SIZE - 1);
                state.is_dirty    = false;
                state.is_selected = false;
                state.status      = "Opened the dictionary";
            }
            else if (path) {
                state.status = "Failed to open the dictionary";
            }
        }

        // the first save needs a path
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (state.file_path[0] != 0) asset_dictionary_save(state.file_path);
            else                         asset_dictionary_save_as();
        }

        ImGui::SameLine();
        if (ImGui::Button("Save As...")) {
            asset_dictionary_save_as();
        }

        ImGui::Text(
            "%s%s",
            state.file_path[0] != 0 ? state.file_path : "(not saved)",
            state.is_dirty          ? " *"            : ""
        );

        if (state.status) {
            ImGui::TextUnformatted(state.status);
        }
    }

    // the values of the next file
    IFB_INLINE void
    asset_dictionary_controls_add(
        void) {

        auto& state = _asset_dictionary;

        ImGui::SeparatorText("Add File");

        ImGui::Combo     ("Type", &state.type, _asset_dictionary_type_names, asset_type_e_count);
        ImGui::InputText ("Name", state.name,  ASSET_DICTIONARY_PATH_SIZE);
        ImGui::InputText ("Path", state.path,  ASSET_DICTIONARY_PATH_SIZE);

        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {

            const char* path = eng_file_dialog_open(
                _asset_dictionary_type_filter_types[state.type],
                _asset_dictionary_type_filter_descs[state.type]
            );

            if (path) {
                strncpy_s(state.path, path, ASSET_DICTIONARY_PATH_SIZE - 1);
                if (state.name[0] == 0) {
                    asset_dictionary_name_from_path();
                }
            }
        }

        if (ImGui::Button("Add")) {

            const bool is_valid = (state.name[0] != 0 && state.path[0] != 0);
            const bool did_add  = (
                is_valid &&
                eng_asset_dictionary_add_file(state.type, state.name, state.path) != INVALID_HANDLE
            );

            // the name and path are cleared for the next file
            if (did_add) {
                state.name[0]  = 0;
                state.path[0]  = 0;
                state.is_dirty = true;
                state.status   = "Added the file";
            }
            else {
                state.status = is_valid
                    ? "Failed to add the file: the name is in use, or the dictionary is full"
                    : "Failed to add the file: it needs a name and a path";
            }
        }
    }

    // every file of a type
    IFB_INLINE void
    asset_dictionary_table(
        const u32 type) {

        auto& state = _asset_dictionary;

        ImGui::SeparatorText(_asset_dictionary_list_names[type]);

        // removing a file changes the list, so it waits until the table is done
        const char* name_to_remove = NULL;

        ImGui::PushID(type);
        if (ImGui::BeginTable("tbl-asset-dictionary", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {

            ImGui::TableSetupColumn ("Name");
            ImGui::TableSetupColumn ("ID",   ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn ("Path");
            ImGui::TableSetupColumn ("",     ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow  ();

            const u32 count = eng_asset_dictionary_get_file_count(type);
            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                asset_dictionary_file_info info;
                if (!eng_asset_dictionary_get_file_info(type, index, info)) continue;

                ImGui::PushID(index);
                ImGui::TableNextRow();

                // the remove button is on top of the row
                ImGui::TableNextColumn();
                const bool is_selected = (state.is_selected && state.selected_id == info.id);
                if (ImGui::Selectable(info.name, is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                    asset_dictionary_select(info.id);
                }

                ImGui::TableNextColumn();
                ImGui::Text("%08X", info.id);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.path);

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Remove")) {
                    name_to_remove = info.name;
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopID();

        if (name_to_remove) {
            const bool did_remove = eng_asset_dictionary_remove_file(name_to_remove);
            if (did_remove) {
                state.is_dirty    = true;
                state.is_selected = false;
            }
            state.status = did_remove
                ? "Removed the file"
                : "Failed to remove the file";
        }
    }

    // the preview is loaded once, when the file is selected
    IFB_INLINE void
    asset_dictionary_select(
        const u32 id) {

        auto& state = _asset_dictionary;

        state.is_selected       = true;
        state.selected_id       = id;
        state.is_preview_loaded = eng_asset_dictionary_load_preview(id, state.preview);
    }

    // the contents of the selected file
    IFB_INLINE void
    asset_dictionary_preview_file(
        void) {

        auto& state = _asset_dictionary;

        ImGui::SeparatorText("Preview");

        if (!state.is_selected) {
            ImGui::TextUnformatted("No file selected");
            return;
        }
        if (!state.is_preview_loaded) {
            ImGui::TextUnformatted("Failed to load the file");
            if (state.file_path[0] == 0) {
                ImGui::TextUnformatted("A relative path can't be found until the dictionary is saved");
            }
            return;
        }

        const asset_dictionary_preview& preview = state.preview;

        if (preview.type == asset_type_e_text) {

            if (preview.is_text_truncated) {
                ImGui::TextUnformatted("The file is too big, this is only the start of it");
            }

            if (ImGui::BeginChild("asset-dictionary-preview-text", ImVec2(0.0f, ASSET_DICTIONARY_PREVIEW_HEIGHT), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
                ImGui::TextUnformatted(preview.text);
            }
            ImGui::EndChild();
        }
        else {

            ImGui::Text("%u x %u", preview.width, preview.height);

            // the image is scaled down to fit, but never up
            const ImVec2 size_avail = ImGui::GetContentRegionAvail();
            const f32    width      = (f32)preview.width;
            const f32    height     = (f32)preview.height;
            const f32    scale_x    = (size_avail.x < width)                       ? (size_avail.x / width)                       : 1.0f;
            const f32    scale_y    = (ASSET_DICTIONARY_PREVIEW_HEIGHT < height)   ? (ASSET_DICTIONARY_PREVIEW_HEIGHT / height)   : 1.0f;
            const f32    scale      = (scale_x < scale_y)                          ? scale_x                                      : scale_y;
            if (scale <= 0.0f) return;

            const ImTextureID texture = (ImTextureID)(intptr_t)preview.texture;
            ImGui::Image(texture, ImVec2(width * scale, height * scale));
        }
    }
};
