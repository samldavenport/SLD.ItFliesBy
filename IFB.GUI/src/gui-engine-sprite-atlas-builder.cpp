#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include "gui-game-asset-dictionary.cpp"
#include "gui-game-asset-database.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 SPRITE_ATLAS_BUILDER_FILTER_SIZE = IFB_CONFIG_ASSET_NAME_SIZE + 1;
    static constexpr u32 SPRITE_ATLAS_BUILDER_NAME_SIZE   = IFB_CONFIG_ASSET_NAME_SIZE + 1;
    static constexpr u32 SPRITE_ATLAS_BUILDER_PATH_SIZE   = IFB_CONFIG_FILE_PATH_SIZE;

    static const char* _sprite_atlas_filter_type = "*.ifbatlas";
    static const char* _sprite_atlas_filter_desc = "Sprite Atlas";
    static const char* _sprite_atlas_ext         = ".ifbatlas";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the sprite list, the window owns the filter and what's being previewed
    // the preview is either the selected image or the last packed atlas
    // the path is the file the atlas was opened from or last saved to, so it can be saved again in place
    struct sprite_atlas_builder_state {
        const char*   status;
        char          status_buffer [128];
        char          filter   [SPRITE_ATLAS_BUILDER_FILTER_SIZE];
        char          selected [SPRITE_ATLAS_BUILDER_NAME_SIZE]; // empty unless an image is selected
        char          path     [SPRITE_ATLAS_BUILDER_PATH_SIZE]; // empty for a new atlas
        bool          is_atlas_selected;
        bool          is_preview_loaded;
        u32           atlas_sprite_count;
        asset_preview preview;
    } static _sprite_atlas_builder;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void sprite_atlas_builder_images      (void);
    IFB_INLINE void sprite_atlas_builder_sprites     (void);
    IFB_INLINE bool sprite_atlas_builder_is_filtered (const char* name);
    IFB_INLINE void sprite_atlas_builder_select      (const char* name);
    IFB_INLINE void sprite_atlas_builder_pack        (const u32 count);
    IFB_INLINE void sprite_atlas_builder_controls    (void);
    IFB_INLINE void sprite_atlas_builder_open        (void);
    IFB_INLINE void sprite_atlas_builder_save        (const u32 count, const bool is_save_as);
    IFB_INLINE void sprite_atlas_builder_preview     (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_sprite_atlas_builder(
        bool& is_open) {

        if (!is_open) return;

        const bool render_window = ImGui::Begin(
            _gui_str->menu_engine_menu_sprite_tools_item_atlas_builder,
            &is_open
        );

        if (render_window) {

            sprite_atlas_builder_controls();

            // the lists get whatever the preview below them doesn't need
            const f32 height_preview = ASSET_DICTIONARY_PREVIEW_HEIGHT + (ImGui::GetFrameHeightWithSpacing() * 4.0f);

            // images on the left, the sprites in the atlas on the right
            if (ImGui::BeginChild("sprite-atlas-builder-lists", ImVec2(0.0f, -height_preview))) {

                if (ImGui::BeginTable("tbl-sprite-atlas-builder", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    sprite_atlas_builder_images();

                    ImGui::TableNextColumn();
                    sprite_atlas_builder_sprites();

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            if (_sprite_atlas_builder.status) {
                ImGui::TextWrapped("%s", _sprite_atlas_builder.status);
            }

            sprite_atlas_builder_preview();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // every image in the runtime database, the ones in the atlas can't be added again
    IFB_INLINE void
    sprite_atlas_builder_images(
        void) {

        auto& state = _sprite_atlas_builder;

        ImGui::SeparatorText("Images");

        const u32 count = eng_asset_runtime_get_count();
        if (count == 0) {
            ImGui::TextWrapped("The runtime database has no assets, or it failed to load");
            return;
        }

        ImGui::InputText("Filter", state.filter, SPRITE_ATLAS_BUILDER_FILTER_SIZE, ImGuiInputTextFlags_CharsUppercase);

        if (ImGui::BeginTable("tbl-sprite-atlas-builder-images", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupScrollFreeze (0, 1);
            ImGui::TableSetupColumn       ("Name");
            ImGui::TableSetupColumn       ("##add", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow        ();

            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                asset_database_info info;
                if (!eng_asset_runtime_get_info(index, info)) continue;
                if (info.type != asset_type_e_image)          continue;
                if (!sprite_atlas_builder_is_filtered(info.name)) continue;

                ImGui::PushID(index);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::Selectable(info.name, strcmp(state.selected, info.name) == 0)) {
                    sprite_atlas_builder_select(info.name);
                }

                ImGui::TableNextColumn();
                const bool is_in_atlas = eng_sprite_atlas_builder_contains(info.name);
                ImGui::BeginDisabled(is_in_atlas);
                if (ImGui::SmallButton("Add")) {
                    state.status = eng_sprite_atlas_builder_add(info.name)
                        ? "Added the sprite"
                        : "Failed to add the sprite, there may be no room to load the image";
                }
                ImGui::EndDisabled();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    // the sprites that will be packed into the atlas, in the order they were added
    IFB_INLINE void
    sprite_atlas_builder_sprites(
        void) {

        auto& state = _sprite_atlas_builder;

        ImGui::SeparatorText("Sprites");

        if (ImGui::Button("Clear")) {
            eng_sprite_atlas_builder_clear();
            state.status = "Cleared the sprites";
        }

        // the total area is the smallest the atlas could be
        const u32 count      = eng_sprite_atlas_builder_get_count();
        u64       total_area = 0;

        ImGui::SameLine();
        ImGui::BeginDisabled(count == 0);
        if (ImGui::Button("Pack")) {
            sprite_atlas_builder_pack(count);
        }
        ImGui::EndDisabled();

        // removing a sprite changes the list, so it waits until the table is done
        bool                      do_remove = false;
        sprite_atlas_builder_info remove_info;

        if (ImGui::BeginTable("tbl-sprite-atlas-builder-sprites", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing()))) {

            ImGui::TableSetupScrollFreeze (0, 1);
            ImGui::TableSetupColumn       ("Name");
            ImGui::TableSetupColumn       ("Width",  ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("Height", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("##remove", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow        ();

            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                sprite_atlas_builder_info info;
                if (!eng_sprite_atlas_builder_get_info(index, info)) continue;

                total_area += (u64)info.width * (u64)info.height;

                ImGui::PushID(index);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::Selectable(info.name, strcmp(state.selected, info.name) == 0)) {
                    sprite_atlas_builder_select(info.name);
                }

                ImGui::TableNextColumn();
                ImGui::Text("%u", info.width);

                ImGui::TableNextColumn();
                ImGui::Text("%u", info.height);

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Remove")) {
                    do_remove   = true;
                    remove_info = info;
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        if (do_remove) {
            state.status = eng_sprite_atlas_builder_remove(remove_info.name)
                ? "Removed the sprite"
                : "Failed to remove the sprite";
        }

        ImGui::Text("%u sprites, %llu pixels", count, total_area);
    }

    // an empty filter matches every name
    IFB_INLINE bool
    sprite_atlas_builder_is_filtered(
        const char* name) {

        const char* filter = _sprite_atlas_builder.filter;
        if (filter[0] == 0) return(true);

        const bool is_match = (strstr(name, filter) != NULL);
        return(is_match);
    }

    // the preview is loaded once, when the image is selected
    IFB_INLINE void
    sprite_atlas_builder_select(
        const char* name) {

        auto& state = _sprite_atlas_builder;

        strncpy_s(state.selected, name, SPRITE_ATLAS_BUILDER_NAME_SIZE - 1);
        state.is_atlas_selected = false;
        state.is_preview_loaded = eng_asset_runtime_load_image_preview(name, state.preview);
    }

    // the atlas is previewed in place of the selected image
    IFB_INLINE void
    sprite_atlas_builder_pack(
        const u32 count) {

        auto& state = _sprite_atlas_builder;

        state.selected[0]        = 0;
        state.is_atlas_selected  = true;
        state.atlas_sprite_count = count;
        state.is_preview_loaded  = eng_sprite_atlas_builder_pack(state.preview);

        state.status = state.is_preview_loaded
            ? "Packed the sprites"
            : "Failed to pack the sprites, they may not fit in the atlas or there's no memory for it";
    }

    // new, open, and save, and the file being edited
    IFB_INLINE void
    sprite_atlas_builder_controls(
        void) {

        auto& state = _sprite_atlas_builder;

        ImGui::SeparatorText("Atlas");

        // a new atlas has nowhere to save to until it's saved as something
        if (ImGui::Button("New")) {
            eng_sprite_atlas_builder_clear();
            state.path[0]           = 0;
            state.selected[0]       = 0;
            state.is_atlas_selected = false;
            state.is_preview_loaded = false;
            state.status            = "Started a new atlas";
        }

        ImGui::SameLine();
        if (ImGui::Button("Open...")) {
            sprite_atlas_builder_open();
        }

        const u32 count = eng_sprite_atlas_builder_get_count();

        ImGui::SameLine();
        ImGui::BeginDisabled(count == 0);
        if (ImGui::Button("Save")) {
            sprite_atlas_builder_save(count, false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Save As...")) {
            sprite_atlas_builder_save(count, true);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::TextUnformatted(state.path[0] != 0 ? state.path : "(new atlas)");
    }

    // the sprites in the file replace the ones in the list, a failed open leaves the list as it was
    // the atlas is packed again right away, so the preview shows it as it'll be saved
    IFB_INLINE void
    sprite_atlas_builder_open(
        void) {

        auto& state = _sprite_atlas_builder;

        const char* dlg_path = eng_file_dialog_open(_sprite_atlas_filter_type, _sprite_atlas_filter_desc);
        if (!dlg_path) return;

        char path[SPRITE_ATLAS_BUILDER_PATH_SIZE];
        strncpy_s(path, dlg_path, SPRITE_ATLAS_BUILDER_PATH_SIZE - 1);

        u32 missing_count = 0;
        if (!eng_sprite_atlas_builder_open(path, missing_count)) {
            state.status = "Failed to open the atlas, it may not be an atlas file or there's no memory for it";
            return;
        }

        strncpy_s(state.path, path, SPRITE_ATLAS_BUILDER_PATH_SIZE - 1);

        const u32 count = eng_sprite_atlas_builder_get_count();
        if (count != 0) {
            sprite_atlas_builder_pack(count);
        }
        else {
            state.selected[0]       = 0;
            state.is_atlas_selected = false;
            state.is_preview_loaded = false;
        }

        if (missing_count == 0) {
            state.status = "Opened the atlas";
            return;
        }

        snprintf(
            state.status_buffer,
            sizeof(state.status_buffer),
            "Opened the atlas, %u sprites weren't added because their images aren't in the runtime database",
            missing_count
        );
        state.status = state.status_buffer;
    }

    // packed first, so the preview shows what was saved
    // saving goes to the file being edited, a new atlas or save as asks where
    IFB_INLINE void
    sprite_atlas_builder_save(
        const u32  count,
        const bool is_save_as) {

        auto& state = _sprite_atlas_builder;

        char path[SPRITE_ATLAS_BUILDER_PATH_SIZE];
        if (is_save_as || state.path[0] == 0) {

            const char* dlg_path = eng_file_dialog_save(_sprite_atlas_filter_type, _sprite_atlas_filter_desc);
            if (!dlg_path) return;

            strncpy_s(path, dlg_path, SPRITE_ATLAS_BUILDER_PATH_SIZE - 1);
            asset_database_add_extension(path, _sprite_atlas_ext);
        }
        else {
            strncpy_s(path, state.path, SPRITE_ATLAS_BUILDER_PATH_SIZE - 1);
        }

        sprite_atlas_builder_pack(count);
        if (!state.is_preview_loaded) return;

        if (!eng_sprite_atlas_builder_save(path)) {
            state.status = "Failed to save the atlas, the file may not be writable or there's no memory for it";
            return;
        }

        strncpy_s(state.path, path, SPRITE_ATLAS_BUILDER_PATH_SIZE - 1);
        state.status = "Saved the atlas";
    }

    // the selected image read back from the runtime database, or the packed atlas
    IFB_INLINE void
    sprite_atlas_builder_preview(
        void) {

        auto& state = _sprite_atlas_builder;

        ImGui::SeparatorText("Preview");

        if (!state.is_atlas_selected && state.selected[0] == 0) {
            ImGui::TextUnformatted("No image selected, and nothing has been packed");
            return;
        }
        if (!state.is_preview_loaded) {
            ImGui::Text("Failed to load %s", state.is_atlas_selected ? "the atlas" : state.selected);
            return;
        }

        if (state.is_atlas_selected) {
            ImGui::Text("Atlas, %u sprites", state.atlas_sprite_count);
        }

        gui_asset_preview("sprite-atlas-builder-preview", state.preview);
    }
};
