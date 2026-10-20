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
    static constexpr u32 SPRITE_ATLAS_INSPECTOR_HOVER_COLOR       = IM_COL32(255, 255, 255, 255);
    static constexpr u32 SPRITE_ATLAS_INSPECTOR_HOVER_FILL_COLOR  = IM_COL32(255, 255, 255, 48);
    static constexpr f32 SPRITE_ATLAS_INSPECTOR_HOVER_THICKNESS   = 1.0f;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the open atlas, the window owns its path, the preview, and which sprites are highlighted on it
    // the table is drawn before the preview, so it uses the sprite hovered on the preview the frame before
    struct sprite_atlas_inspector_state {
        const char*   status;
        char          path [SPRITE_ATLAS_INSPECTOR_PATH_SIZE]; // empty unless an atlas is open
        bool          is_preview_loaded;
        bool          scroll_to_selected;                      // set when a sprite is selected on the preview
        u32           selected_index;                          // INVALID_INDEX unless a sprite is selected
        u32           hovered_preview_index;                   // the sprite under the mouse on the preview
        u32           hovered_row_index;                       // the sprite under the mouse in the table
        asset_preview preview;
    } static _sprite_atlas_inspector = { NULL, {0}, false, false, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, {0} };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void sprite_atlas_inspector_controls (void);
    IFB_INLINE void sprite_atlas_inspector_sprites  (const f32 height_preview);
    IFB_INLINE void sprite_atlas_inspector_open     (void);
    IFB_INLINE void sprite_atlas_inspector_close    (void);
    IFB_INLINE void sprite_atlas_inspector_preview  (void);
    IFB_INLINE void sprite_atlas_inspector_select   (const u32 index);
    IFB_INLINE void sprite_atlas_inspector_tooltip  (const u32 index);
    IFB_INLINE u32  sprite_atlas_inspector_hit_test (const ImVec2& image_min, const f32 scale, const ImVec2& mouse);
    IFB_INLINE void sprite_atlas_inspector_draw     (const ImVec2& image_min, const f32 scale, const u32 index, const u32 color, const f32 thickness, const u32 fill_color);

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
    // the hovered one is highlighted on the preview, and the row of the sprite hovered on the preview is highlighted here
    IFB_INLINE void
    sprite_atlas_inspector_sprites(
        const f32 height_preview) {

        auto& state = _sprite_atlas_inspector;

        state.hovered_row_index = INVALID_INDEX;

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

                if (state.hovered_preview_index == index) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImGuiCol_HeaderHovered));
                }

                ImGui::TableNextColumn();
                const bool is_selected = (state.selected_index == index);
                if (ImGui::Selectable(sprite.name, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    sprite_atlas_inspector_select(index);
                }
                if (ImGui::IsItemHovered()) {
                    state.hovered_row_index = index;
                }

                // a sprite picked on the preview may be scrolled out of view
                if (is_selected && state.scroll_to_selected) {
                    ImGui::SetScrollHereY(0.5f);
                    state.scroll_to_selected = false;
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
        state.preview               = preview;
        state.is_preview_loaded     = true;
        state.scroll_to_selected    = false;
        state.selected_index        = INVALID_INDEX;
        state.hovered_preview_index = INVALID_INDEX;
        state.status                = "Opened the atlas";
    }

    IFB_INLINE void
    sprite_atlas_inspector_close(
        void) {

        auto& state = _sprite_atlas_inspector;

        eng_sprite_atlas_inspector_close();

        state.path[0]               = 0;
        state.is_preview_loaded     = false;
        state.scroll_to_selected    = false;
        state.selected_index        = INVALID_INDEX;
        state.hovered_preview_index = INVALID_INDEX;
        state.status                = "Closed the atlas";
    }

    // the whole atlas, read back from the file
    // the sprite under the mouse is highlighted, clicking it selects it
    IFB_INLINE void
    sprite_atlas_inspector_preview(
        void) {

        auto& state = _sprite_atlas_inspector;

        state.hovered_preview_index = INVALID_INDEX;

        ImGui::SeparatorText("Preview");

        if (!state.is_preview_loaded) {
            ImGui::TextUnformatted("No atlas open");
            return;
        }

        // the image is the last item, so its rect is where the preview was drawn
        if (!gui_asset_preview("sprite-atlas-inspector-preview", state.preview)) return;

        const ImVec2 image_min = ImGui::GetItemRectMin();
        const ImVec2 image_max = ImGui::GetItemRectMax();
        const f32    scale     = (image_max.x - image_min.x) / (f32)state.preview.width;

        if (ImGui::IsItemHovered()) {

            state.hovered_preview_index = sprite_atlas_inspector_hit_test(image_min, scale, ImGui::GetMousePos());

            if (state.hovered_preview_index != INVALID_INDEX && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                sprite_atlas_inspector_select(state.hovered_preview_index);
                state.scroll_to_selected = (state.selected_index != INVALID_INDEX);
            }

            sprite_atlas_inspector_tooltip(state.hovered_preview_index);
        }

        // a row hovered in the table is highlighted the same way
        const u32 hovered_index = (state.hovered_preview_index != INVALID_INDEX)
            ? state.hovered_preview_index
            : state.hovered_row_index;

        sprite_atlas_inspector_draw(image_min, scale, hovered_index,        SPRITE_ATLAS_INSPECTOR_HOVER_COLOR,   SPRITE_ATLAS_INSPECTOR_HOVER_THICKNESS,   SPRITE_ATLAS_INSPECTOR_HOVER_FILL_COLOR);
        sprite_atlas_inspector_draw(image_min, scale, state.selected_index, SPRITE_ATLAS_INSPECTOR_OUTLINE_COLOR, SPRITE_ATLAS_INSPECTOR_OUTLINE_THICKNESS, 0);
    }

    // selecting the selected sprite again clears it, the same as the table rows
    IFB_INLINE void
    sprite_atlas_inspector_select(
        const u32 index) {

        auto& state = _sprite_atlas_inspector;

        state.selected_index = (state.selected_index == index)
            ? INVALID_INDEX
            : index;
    }

    // the same info as the sprite's row, next to the mouse
    IFB_INLINE void
    sprite_atlas_inspector_tooltip(
        const u32 index) {

        if (index == INVALID_INDEX) return;

        sprite_atlas_sprite_info sprite;
        if (!eng_sprite_atlas_inspector_get_sprite(index, sprite)) return;

        if (ImGui::BeginTooltip()) {
            ImGui::TextUnformatted (sprite.name);
            ImGui::Separator       ();
            ImGui::Text            ("Index     %u",       index);
            ImGui::Text            ("Handle    %08X",     sprite.hnd.val);
            ImGui::Text            ("Position  %u, %u",   sprite.x,     sprite.y);
            ImGui::Text            ("Size      %u x %u",  sprite.width, sprite.height);
            ImGui::EndTooltip();
        }
    }

    // the mouse is in screen space, so it's scaled back to atlas pixels
    // the sprites don't overlap, so the first one that has the pixel is the only one
    IFB_INLINE u32
    sprite_atlas_inspector_hit_test(
        const ImVec2& image_min,
        const f32     scale,
        const ImVec2& mouse) {

        if (scale <= 0.0f) return(INVALID_INDEX);

        const f32 atlas_x = (mouse.x - image_min.x) / scale;
        const f32 atlas_y = (mouse.y - image_min.y) / scale;
        if (atlas_x < 0.0f || atlas_y < 0.0f) return(INVALID_INDEX);

        sprite_atlas_info info;
        if (!eng_sprite_atlas_inspector_get_info(info)) return(INVALID_INDEX);

        for (
            u32 index = 0;
                index < info.sprite_count;
              ++index) {

            sprite_atlas_sprite_info sprite;
            if (!eng_sprite_atlas_inspector_get_sprite(index, sprite)) continue;

            const bool is_inside = (
                atlas_x >= (f32)sprite.x                   &&
                atlas_y >= (f32)sprite.y                   &&
                atlas_x <  (f32)(sprite.x + sprite.width)  &&
                atlas_y <  (f32)(sprite.y + sprite.height)
            );
            if (is_inside) return(index);
        }

        return(INVALID_INDEX);
    }

    // the preview may be scaled down, so the sprite's rect is scaled the same way
    // a fill color of zero draws only the outline
    IFB_INLINE void
    sprite_atlas_inspector_draw(
        const ImVec2& image_min,
        const f32     scale,
        const u32     index,
        const u32     color,
        const f32     thickness,
        const u32     fill_color) {

        if (index == INVALID_INDEX) return;

        sprite_atlas_sprite_info sprite;
        if (!eng_sprite_atlas_inspector_get_sprite(index, sprite)) return;

        const ImVec2 rect_min(
            image_min.x + ((f32)sprite.x * scale),
            image_min.y + ((f32)sprite.y * scale)
        );
        const ImVec2 rect_max(
            image_min.x + ((f32)(sprite.x + sprite.width)  * scale),
            image_min.y + ((f32)(sprite.y + sprite.height) * scale)
        );

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        if (fill_color != 0) {
            draw_list->AddRectFilled(rect_min, rect_max, fill_color);
        }
        draw_list->AddRect(rect_min, rect_max, color, 0.0f, 0, thickness);
    }
};
