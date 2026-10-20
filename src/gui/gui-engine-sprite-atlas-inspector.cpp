#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb-types.hpp"
#include "imgui.h"
#include "gui-game-asset-dictionary.cpp"
#include "gui-engine-sprite-atlas-builder.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 SPRITE_ATLAS_INSPECTOR_PATH_SIZE         = IFB_CONFIG_FILE_PATH_SIZE;
    static constexpr u32 SPRITE_ATLAS_INSPECTOR_OUTLINE_COLOR     = IM_COL32(255, 255, 0, 255);
    static constexpr f32 SPRITE_ATLAS_INSPECTOR_OUTLINE_THICKNESS = 2.0f;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the open atlas, the window owns its path, the preview, and which sprite is outlined on it
    struct sprite_atlas_inspector_state {
        const char*   status;
        char          path [SPRITE_ATLAS_INSPECTOR_PATH_SIZE]; // empty unless an atlas is open
        bool          is_preview_loaded;
        u32           selected_index;                          // INVALID_INDEX unless a sprite is selected
        asset_preview preview;
    } static _sprite_atlas_inspector = { NULL, {0}, false, INVALID_INDEX, {0} };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void sprite_atlas_inspector_controls (void);
    IFB_INLINE void sprite_atlas_inspector_sprites  (const f32 height_preview);
    IFB_INLINE void sprite_atlas_inspector_open     (void);
    IFB_INLINE void sprite_atlas_inspector_close    (void);
    IFB_INLINE void sprite_atlas_inspector_preview  (void);
    IFB_INLINE void sprite_atlas_inspector_outline  (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_sprite_atlas_inspector(
        bool& is_open) {

        if (!is_open) return;

        const bool render_window = ImGui::Begin(
            _gui_str->menu_engine_menu_sprite_tools_item_atlas_inspector,
            &is_open
        );

        if (render_window) {

            // the sprite list gets whatever the preview below it doesn't need
            const f32 height_preview = ASSET_DICTIONARY_PREVIEW_HEIGHT + (ImGui::GetFrameHeightWithSpacing() * 3.0f);

            sprite_atlas_inspector_controls ();
            sprite_atlas_inspector_sprites  (height_preview);
            sprite_atlas_inspector_preview  ();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // open and close the atlas, and what's in it
    IFB_INLINE void
    sprite_atlas_inspector_controls(
        void) {

        auto& state = _sprite_atlas_inspector;

        ImGui::SeparatorText("Atlas");

        if (ImGui::Button("Open...")) {
            sprite_atlas_inspector_open();
        }

        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            sprite_atlas_inspector_close();
        }

        sprite_atlas_info info;
        if (eng_sprite_atlas_inspector_get_info(info)) {
            ImGui::TextWrapped("%s", state.path);
            ImGui::Text("%u x %u pixels, %u sprites", info.width, info.height, info.sprite_count);
        }
        else {
            ImGui::TextUnformatted("(none)");
        }

        if (state.status) {
            ImGui::TextWrapped("%s", state.status);
        }
    }

    // every sprite in the atlas, in the order they are stored
    // the selected one is outlined on the preview, selecting it again clears it
    IFB_INLINE void
    sprite_atlas_inspector_sprites(
        const f32 height_preview) {

        auto& state = _sprite_atlas_inspector;

        ImGui::SeparatorText("Sprites");

        sprite_atlas_info info;
        const u32 count = eng_sprite_atlas_inspector_get_info(info) ? info.sprite_count : 0;

        if (ImGui::BeginTable("tbl-sprite-atlas-inspector", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.0f, -height_preview))) {

            ImGui::TableSetupScrollFreeze (0, 1);
            ImGui::TableSetupColumn       ("Name");
            ImGui::TableSetupColumn       ("Handle", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("X",      ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("Y",      ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("Width",  ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn       ("Height", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow        ();

            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                sprite_atlas_sprite_info sprite;
                if (!eng_sprite_atlas_inspector_get_sprite(index, sprite)) continue;

                ImGui::PushID(index);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                const bool is_selected = (state.selected_index == index);
                if (ImGui::Selectable(sprite.name, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    state.selected_index = is_selected ? INVALID_INDEX : index;
                }

                ImGui::TableNextColumn();
                ImGui::Text("%08X", sprite.hnd.val);

                ImGui::TableNextColumn();
                ImGui::Text("%u", sprite.x);

                ImGui::TableNextColumn();
                ImGui::Text("%u", sprite.y);

                ImGui::TableNextColumn();
                ImGui::Text("%u", sprite.width);

                ImGui::TableNextColumn();
                ImGui::Text("%u", sprite.height);

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    // a failed open leaves the open atlas as it was
    IFB_INLINE void
    sprite_atlas_inspector_open(
        void) {

        auto& state = _sprite_atlas_inspector;

        const char* path = eng_file_dialog_open(_sprite_atlas_filter_type, _sprite_atlas_filter_desc);
        if (!path) return;

        asset_preview preview;
        if (!eng_sprite_atlas_inspector_open(path, preview)) {
            state.status = "Failed to open the atlas, it may not be an atlas file or there's no memory for it";
            return;
        }

        strncpy_s(state.path, path, SPRITE_ATLAS_INSPECTOR_PATH_SIZE - 1);
        state.preview           = preview;
        state.is_preview_loaded = true;
        state.selected_index    = INVALID_INDEX;
        state.status            = "Opened the atlas";
    }

    IFB_INLINE void
    sprite_atlas_inspector_close(
        void) {

        auto& state = _sprite_atlas_inspector;

        eng_sprite_atlas_inspector_close();

        state.path[0]           = 0;
        state.is_preview_loaded = false;
        state.selected_index    = INVALID_INDEX;
        state.status            = "Closed the atlas";
    }

    // the whole atlas, read back from the file
    IFB_INLINE void
    sprite_atlas_inspector_preview(
        void) {

        auto& state = _sprite_atlas_inspector;

        ImGui::SeparatorText("Preview");

        if (!state.is_preview_loaded) {
            ImGui::TextUnformatted("No atlas open");
            return;
        }

        if (gui_asset_preview("sprite-atlas-inspector-preview", state.preview)) {
            sprite_atlas_inspector_outline();
        }
    }

    // the preview may be scaled down, so the sprite's rect is scaled the same way
    // this has to follow the preview, it draws over the last item
    IFB_INLINE void
    sprite_atlas_inspector_outline(
        void) {

        auto& state = _sprite_atlas_inspector;

        if (state.selected_index == INVALID_INDEX) return;

        sprite_atlas_sprite_info sprite;
        if (!eng_sprite_atlas_inspector_get_sprite(state.selected_index, sprite)) return;

        const ImVec2 image_min = ImGui::GetItemRectMin();
        const ImVec2 image_max = ImGui::GetItemRectMax();
        const f32    scale     = (image_max.x - image_min.x) / (f32)state.preview.width;

        const ImVec2 outline_min(
            image_min.x + ((f32)sprite.x * scale),
            image_min.y + ((f32)sprite.y * scale)
        );
        const ImVec2 outline_max(
            image_min.x + ((f32)(sprite.x + sprite.width)  * scale),
            image_min.y + ((f32)(sprite.y + sprite.height) * scale)
        );

        ImGui::GetWindowDrawList()->AddRect(
            outline_min,
            outline_max,
            SPRITE_ATLAS_INSPECTOR_OUTLINE_COLOR,
            0.0f,
            0,
            SPRITE_ATLAS_INSPECTOR_OUTLINE_THICKNESS
        );
    }
};
