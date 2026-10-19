#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb-types.hpp"
#include "imgui.h"
#include "gui-game-asset-dictionary.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 SPRITE_ATLAS_BUILDER_FILTER_SIZE = IFB_CONFIG_ASSET_NAME_SIZE + 1;
    static constexpr u32 SPRITE_ATLAS_BUILDER_NAME_SIZE   = IFB_CONFIG_ASSET_NAME_SIZE + 1;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the sprite list, the window owns the filter and the selected image
    struct sprite_atlas_builder_state {
        const char*   status;
        char          filter   [SPRITE_ATLAS_BUILDER_FILTER_SIZE];
        char          selected [SPRITE_ATLAS_BUILDER_NAME_SIZE]; // empty until an image is selected
        bool          is_preview_loaded;
        asset_preview preview;
    } static _sprite_atlas_builder;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void sprite_atlas_builder_images      (void);
    IFB_INLINE void sprite_atlas_builder_sprites     (void);
    IFB_INLINE bool sprite_atlas_builder_is_filtered (const char* name);
    IFB_INLINE void sprite_atlas_builder_select      (const char* name);
    IFB_INLINE void sprite_atlas_builder_preview     (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_sprite_atlas_builder(
        bool& is_open) {

        if (!is_open) return;

        const bool render_window = ImGui::Begin(
            _gui_str->menu_engine_item_sprite_atlas_builder,
            &is_open
        );

        if (render_window) {

            // the lists get whatever the preview below them doesn't need
            const f32 height_preview = ASSET_DICTIONARY_PREVIEW_HEIGHT + (ImGui::GetFrameHeightWithSpacing() * 3.0f);

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
        state.is_preview_loaded = eng_asset_runtime_load_image_preview(name, state.preview);
    }

    // the selected image, read back from the runtime database
    IFB_INLINE void
    sprite_atlas_builder_preview(
        void) {

        auto& state = _sprite_atlas_builder;

        ImGui::SeparatorText("Preview");

        if (state.selected[0] == 0) {
            ImGui::TextUnformatted("No image selected");
            return;
        }
        if (!state.is_preview_loaded) {
            ImGui::Text("Failed to load %s", state.selected);
            return;
        }

        gui_asset_preview("sprite-atlas-builder-preview", state.preview);
    }
};
