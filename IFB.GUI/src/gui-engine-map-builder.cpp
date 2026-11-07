#pragma once

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // the builder's copy of a map gets its own name,
    // so it doesn't share a handle with the game's copy of the same file
    static const char* MAP_BUILDER_MAP_NAME = "MAP-BUILDER";

    // the game's camera sits 0.839 back from its target and 0.1 above it
    static constexpr f32 MAP_BUILDER_GAME_DISTANCE = 0.8450f;
    static constexpr f32 MAP_BUILDER_GAME_PITCH    = 0.1186f; // radians, atan(0.1 / 0.839)

    static constexpr f32 MAP_BUILDER_PITCH_MIN     = 0.02f;
    static constexpr f32 MAP_BUILDER_PITCH_MAX     = 1.55f;
    static constexpr f32 MAP_BUILDER_DISTANCE_MIN  = 0.20f;
    static constexpr f32 MAP_BUILDER_DISTANCE_MAX  = 60.0f;
    static constexpr f32 MAP_BUILDER_ORBIT_SPEED   = 0.008f; // radians per pixel
    static constexpr f32 MAP_BUILDER_ZOOM_STEP     = 0.88f;  // distance scale per wheel notch

    static constexpr f32 MAP_BUILDER_PALETTE_WIDTH = 200.0f;
    static constexpr f32 MAP_BUILDER_THUMB_SIZE    = 48.0f;

    static constexpr u32 MAP_BUILDER_NAME_SIZE     = config::asset_name_size + 1;
    static constexpr u32 MAP_BUILDER_MAP_NAME_SIZE = 16; // a map's name is up to 15 characters
    static constexpr u32 MAP_BUILDER_PATH_SIZE     = IFB_CONFIG_FILE_PATH_SIZE;

    static const char* _map_builder_filter_type = "*.json";
    static const char* _map_builder_filter_desc = "Map File";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    enum map_builder_tool_e {
        map_builder_tool_e_move  = 0, // left drag pans
        map_builder_tool_e_brush = 1, // left click and drag paints tiles
        map_builder_tool_e_chunk = 2, // left drag adds a chunk, left click selects one
        map_builder_tool_e_wall  = 3, // left drag places a straight run of walls, ctrl erases them
    };

    // an orbit camera around a target, turned into an origin and target for the map view
    struct map_builder_camera {
        vec3 target;
        f32  yaw;      // 0 looks down -z, like the game
        f32  pitch;    // up from the ground
        f32  distance;
    };

    struct map_builder_hover {
        bool          is_valid;    // the mouse is over the map
        bool          is_on_chunk; // and over a chunk tile, which can be painted
        s32           row;
        s32           col;
        f32           floor_row;   // where the mouse is on the floor, in tiles, for finding the nearest edge
        f32           floor_col;
        map_tile_info tile;
    };

    // a tile edge, which is where walls go
    struct map_builder_edge {
        map_wall_orientation_e orientation;
        s32                    row;
        s32                    col;
    };

    // the brush paints one sprite from the palette's atlas, or clears the sprite
    struct map_builder_brush {
        hnd_sprite_atlas h_atlas = {INVALID_HANDLE};
        char             atlas_name  [MAP_BUILDER_NAME_SIZE];
        u32              i_sprite;   // INVALID_INDEX clears the sprite
        char             sprite_name [MAP_BUILDER_NAME_SIZE];
    };

    struct map_builder_state {
        hnd_map            h_map = {INVALID_HANDLE};
        char               asset_name [MAP_BUILDER_NAME_SIZE];
        char               map_name   [MAP_BUILDER_MAP_NAME_SIZE]; // the name the map is saved with
        char               save_path  [MAP_BUILDER_PATH_SIZE];     // empty until the map is saved
        char               status     [MAP_BUILDER_PATH_SIZE + 128];
        map_builder_camera camera;
        map_builder_hover  hover;
        map_builder_brush  brush;
        map_builder_tool_e tool;
        u32                rows;
        u32                cols;
        bool               show_grid;
        bool               is_modified;
        u32                selected_chunk;    // INVALID_INDEX when no chunk is selected
        u32                new_chunk_color;   // map_color_e for chunks the chunk tool adds
        bool               is_dragging_chunk;
        s32                drag_row;          // the tile the chunk drag started on
        s32                drag_col;
        s32                drag_end_row;      // the last tile it was dragged over
        s32                drag_end_col;
        u32                wall_color;        // map_color_e for walls the wall tool places
        bool               is_wall_erase;     // the wall tool erases instead of placing
        bool               is_dragging_wall;
        bool               is_drag_erasing;   // this drag erases, from the erase mode or ctrl
        map_builder_edge   wall_start;        // the edge the wall drag started on, it sets the line
        s32                wall_end_step;     // how far along the line the drag is, a column for row walls, a row for column walls
    } static _map_builder;


    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void map_builder_toolbar          (void);
    IFB_INLINE void map_builder_palette          (void);
    IFB_INLINE void map_builder_palette_atlas    (void);
    IFB_INLINE void map_builder_select_atlas     (const char* atlas_name);
    IFB_INLINE void map_builder_load             (const char* asset_name);
    IFB_INLINE void map_builder_save             (const bool is_save_as);
    IFB_INLINE void map_builder_unload           (void);
    IFB_INLINE void map_builder_frame_map        (void);
    IFB_INLINE void map_builder_game_view        (void);
    IFB_INLINE void map_builder_apply_camera     (void);
    IFB_INLINE void map_builder_viewport         (void);
    IFB_INLINE void map_builder_update_camera    (const ImVec2& image_min, const ImVec2& size, const bool is_active, const bool is_hovered);
    IFB_INLINE void map_builder_update_hover     (const ImVec2& image_min, const ImVec2& size, const bool is_hovered);
    IFB_INLINE void map_builder_paint            (void);
    IFB_INLINE bool map_builder_color_combo      (const char* label, u32& color);
    IFB_INLINE void map_builder_chunk_panel      (void);
    IFB_INLINE void map_builder_delete_selected_chunk (void);
    IFB_INLINE void map_builder_chunk_drag_rect  (u32& row_min, u32& col_min, u32& row_max, u32& col_max);
    IFB_INLINE bool map_builder_chunk_rect_is_free (const u32 row_min, const u32 col_min, const u32 row_max, const u32 col_max);
    IFB_INLINE void map_builder_chunk_tool       (const bool is_active);
    IFB_INLINE void map_builder_draw_floor_rect  (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size, const u32 row_min, const u32 col_min, const u32 row_max, const u32 col_max, const ImU32 fill, const ImU32 outline, const f32 thickness);
    IFB_INLINE void map_builder_draw_chunks      (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size);
    IFB_INLINE void map_builder_wall_panel       (void);
    IFB_INLINE bool map_builder_hovered_edge     (map_builder_edge& edge);
    IFB_INLINE void map_builder_wall_drag_range  (s32& step_min, s32& step_max);
    IFB_INLINE map_builder_edge map_builder_wall_drag_edge (const s32 step);
    IFB_INLINE void map_builder_wall_tool        (const bool is_active);
    IFB_INLINE void map_builder_draw_edge        (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size, const map_builder_edge& edge, const ImU32 fill, const ImU32 outline);
    IFB_INLINE void map_builder_draw_walls       (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size);
    IFB_INLINE void map_builder_wall_text        (void);
    IFB_INLINE void map_builder_draw_hover       (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size);
    IFB_INLINE void map_builder_hover_text       (void);
    IFB_INLINE bool map_builder_get_map_origin   (vec3& origin);
    IFB_INLINE bool map_builder_get_sprite_name  (const hnd_sprite_atlas h_atlas, const u32 i_sprite, char* name);
    IFB_INLINE bool map_builder_screen_to_floor  (const ImVec2& screen, const ImVec2& image_min, const ImVec2& size, const f32 plane_y, f32& x, f32& z);
    IFB_INLINE bool map_builder_world_to_screen  (const mat4& xform, const vec3& world, const ImVec2& image_min, const ImVec2& size, ImVec2& screen);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_map_builder(
        bool& is_open) {

        if (!is_open) return;

        auto& state = _map_builder;

        // start where the game's camera starts, until a map is framed
        static bool is_initialized = false;
        if (!is_initialized) {
            is_initialized         = true;
            state.camera.target    = { 0.0f, 0.2f, 0.1f };
            state.camera.yaw       = 0.0f;
            state.camera.pitch     = MAP_BUILDER_GAME_PITCH;
            state.camera.distance  = MAP_BUILDER_GAME_DISTANCE;
            state.tool             = map_builder_tool_e_move;
            state.selected_chunk   = INVALID_INDEX;
            state.new_chunk_color  = (u32)map_color_e_gray_light;
            state.is_dragging_chunk = false;
            state.wall_color       = (u32)map_color_e_gray_dark;
            state.is_wall_erase    = false;
            state.is_dragging_wall = false;
            state.show_grid        = false;
            state.is_modified      = false;
            state.status[0]        = 0;
            state.asset_name[0]    = 0;
            state.brush.h_atlas    = INVALID_HANDLE;
            state.brush.i_sprite   = INVALID_INDEX;
            state.brush.atlas_name[0]  = 0;
            state.brush.sprite_name[0] = 0;
        }

        ImGui::SetNextWindowSize(ImVec2(1000.0f, 640.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(_gui_str->menu_engine_menu_tools_item_map_builder, &is_open)) {

            map_builder_toolbar();

            if (ImGui::BeginChild("##map-builder-palette", ImVec2(MAP_BUILDER_PALETTE_WIDTH, 0.0f), ImGuiChildFlags_Border)) {
                map_builder_palette();
            }
            ImGui::EndChild();

            ImGui::SameLine();
            if (ImGui::BeginChild("##map-builder-view", ImVec2(0.0f, 0.0f))) {
                map_builder_viewport();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    map_builder_toolbar(
        void) {

        auto& state = _map_builder;

        // maps are text assets, so any text asset in the runtime database can be picked
        const char* preview = (state.asset_name[0] != 0) ? state.asset_name : "Select a map asset";
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::BeginCombo("##map-asset", preview)) {
            const u32 count = eng_asset_runtime_get_count();
            for (u32 i = 0; i < count; ++i) {
                asset_database_info info;
                if (!eng_asset_runtime_get_info(i, info))   continue;
                if (info.type != asset_type_e_text)         continue;

                const bool is_selected = (strncmp(info.name, state.asset_name, MAP_BUILDER_NAME_SIZE) == 0);
                if (ImGui::Selectable(info.name, is_selected)) {
                    strncpy_s(state.asset_name, info.name, MAP_BUILDER_NAME_SIZE - 1);
                }
            }
            ImGui::EndCombo();
        }

        // loading again throws away anything painted since the last load
        ImGui::SameLine();
        ImGui::BeginDisabled(state.asset_name[0] == 0);
        if (ImGui::Button(state.is_modified ? "Reload" : "Load")) {
            map_builder_load(state.asset_name);
        }
        if (state.is_modified && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("Throws away the changes to this map");
        }
        ImGui::EndDisabled();

        // the map's name goes in the file, it's what eng_map_load names the map
        ImGui::SameLine();
        ImGui::BeginDisabled(state.h_map == INVALID_HANDLE);
        ImGui::SetNextItemWidth(130.0f);
        ImGui::InputTextWithHint("##map-name", "map name", state.map_name, MAP_BUILDER_MAP_NAME_SIZE);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("The name saved in the file, up to 15 characters");
        }

        // saving asks where the first time, then saves there until another map is loaded
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            map_builder_save(false);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (state.save_path[0] != 0) ImGui::SetTooltip("Save to %s", state.save_path);
            else                         ImGui::SetTooltip("Pick where to save the map");
        }
        ImGui::SameLine();
        if (ImGui::Button("Save As...")) {
            map_builder_save(true);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(state.h_map == INVALID_HANDLE);
        if (ImGui::Button("Frame Map")) map_builder_frame_map();
        ImGui::SameLine();
        if (ImGui::Button("Game View")) map_builder_game_view();
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Checkbox("Grid", &state.show_grid)) {
            eng_map_view_set_grid_visible(state.show_grid);
        }

        // tools
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        int tool = (int)state.tool;
        ImGui::RadioButton("Move",  &tool, (int)map_builder_tool_e_move);
        ImGui::SameLine();
        ImGui::RadioButton("Brush", &tool, (int)map_builder_tool_e_brush);
        ImGui::SameLine();
        ImGui::RadioButton("Chunk", &tool, (int)map_builder_tool_e_chunk);
        ImGui::SameLine();
        ImGui::RadioButton("Wall",  &tool, (int)map_builder_tool_e_wall);
        state.tool = (map_builder_tool_e)tool;

        // what's loaded, what's under the mouse, and how to move around
        if (state.status[0] != 0) {
            ImGui::TextUnformatted(state.status);
        }
        else if (state.h_map == INVALID_HANDLE) {
            ImGui::TextDisabled("No map loaded");
        }
        else {
            ImGui::Text("%s  %u x %u", state.asset_name, state.rows, state.cols);
            if (state.is_modified) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "Modified, not saved");
            }
        }

        ImGui::SameLine();
        if (state.tool == map_builder_tool_e_wall) map_builder_wall_text();
        else                                       map_builder_hover_text();

        ImGui::SameLine();
        if (state.tool == map_builder_tool_e_brush) {
            ImGui::TextDisabled("   left: paint   middle drag: pan   right drag: orbit   wheel: zoom");
        }
        else if (state.tool == map_builder_tool_e_chunk) {
            ImGui::TextDisabled("   left drag: add chunk   left click: select   delete: remove   middle drag: pan   right drag: orbit");
        }
        else if (state.tool == map_builder_tool_e_wall) {
            ImGui::TextDisabled("   left drag: place walls   ctrl + left drag: erase   middle drag: pan   right drag: orbit");
        }
        else {
            ImGui::TextDisabled("   left or middle drag: pan   right drag: orbit   wheel: zoom");
        }
    }

    IFB_INLINE void
    map_builder_palette(
        void) {

        auto& state = _map_builder;
        auto& brush = state.brush;

        // the chunk tool fills chunks with the brush, so the sprites stay below it
        if (state.tool == map_builder_tool_e_chunk) {
            map_builder_chunk_panel();
            ImGui::SeparatorText("Brush");
        }
        else if (state.tool == map_builder_tool_e_wall) {
            map_builder_wall_panel();
            ImGui::SeparatorText("Brush");
        }

        map_builder_palette_atlas();

        // the tile shader only draws from the active atlas
        const hnd_sprite_atlas h_active = eng_sprite_atlas_get_active();
        if (brush.h_atlas != INVALID_HANDLE && brush.h_atlas != h_active) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Not the active atlas,");
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "it won't draw right");
        }

        ImGui::Separator();

        // clearing a tile is the -1 in a map file, the tile shows its chunk's color
        if (ImGui::Selectable("No sprite", brush.i_sprite == INVALID_INDEX)) {
            brush.i_sprite       = INVALID_INDEX;
            brush.sprite_name[0] = 0;
            if (state.tool == map_builder_tool_e_move) state.tool = map_builder_tool_e_brush;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Paints the chunk's color, like -1 in a map file");
        }

        ImGui::Separator();

        sprite_atlas_info atlas_info;
        if (brush.h_atlas == INVALID_HANDLE || !eng_sprite_atlas_get_info(brush.h_atlas, atlas_info)) {
            ImGui::TextDisabled("No atlas");
            return;
        }

        const u32 texture = eng_sprite_atlas_get_texture(brush.h_atlas);
        if (texture == 0 || atlas_info.width == 0 || atlas_info.height == 0) {
            ImGui::TextDisabled("The atlas isn't uploaded");
            return;
        }

        // as many thumbnails across as fit
        const ImGuiStyle& style    = ImGui::GetStyle();
        const f32         cell     = MAP_BUILDER_THUMB_SIZE + (style.FramePadding.x * 2.0f) + style.ItemSpacing.x;
        const f32         width    = ImGui::GetContentRegionAvail().x;
        u32               per_row  = (u32)((width + style.ItemSpacing.x) / cell);
        if (per_row == 0) per_row = 1;

        for (u32 i_sprite = 0; i_sprite < atlas_info.sprite_count; ++i_sprite) {

            sprite_info info;
            if (!eng_sprite_atlas_get_sprite_info(brush.h_atlas, i_sprite, info)) continue;

            char name[MAP_BUILDER_NAME_SIZE];
            map_builder_get_sprite_name(brush.h_atlas, i_sprite, name);

            // the top row of the atlas is v = 0
            const ImVec2 uv0 = ImVec2(
                (f32)info.pixel_x / (f32)atlas_info.width,
                (f32)info.pixel_y / (f32)atlas_info.height);
            const ImVec2 uv1 = ImVec2(
                (f32)(info.pixel_x + info.pixel_width)  / (f32)atlas_info.width,
                (f32)(info.pixel_y + info.pixel_height) / (f32)atlas_info.height);

            const bool is_selected = (brush.i_sprite == i_sprite);
            if (is_selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }

            ImGui::PushID((int)i_sprite);
            if (ImGui::ImageButton("##sprite", (ImTextureID)(intptr_t)texture, ImVec2(MAP_BUILDER_THUMB_SIZE, MAP_BUILDER_THUMB_SIZE), uv0, uv1)) {
                brush.i_sprite = i_sprite;
                strncpy_s(brush.sprite_name, name, MAP_BUILDER_NAME_SIZE - 1);
                if (state.tool == map_builder_tool_e_move) state.tool = map_builder_tool_e_brush;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", name);
            }
            ImGui::PopID();

            if (is_selected) {
                ImGui::PopStyleColor();
            }

            if (((i_sprite + 1) % per_row) != 0 && (i_sprite + 1) < atlas_info.sprite_count) {
                ImGui::SameLine();
            }
        }

        ImGui::Separator();
        if (brush.i_sprite == INVALID_INDEX) ImGui::TextDisabled("Brush: no sprite");
        else                                 ImGui::Text("Brush: %s", brush.sprite_name);
    }

    // the palette's atlas, picked from the atlases in the runtime database
    IFB_INLINE void
    map_builder_palette_atlas(
        void) {

        auto& brush = _map_builder.brush;

        // start on the active atlas, it's the one the map draws with
        if (brush.h_atlas == INVALID_HANDLE) {
            const hnd_sprite_atlas h_active = eng_sprite_atlas_get_active();
            const u32 count = eng_asset_runtime_get_count();
            for (u32 i = 0; i < count && h_active != INVALID_HANDLE; ++i) {
                asset_database_info info;
                if (!eng_asset_runtime_get_info(i, info))        continue;
                if (info.type != asset_type_e_sprite_atlas)      continue;
                if (eng_sprite_atlas_load(info.name) == h_active) {
                    map_builder_select_atlas(info.name);
                    break;
                }
            }
        }

        const char* preview = (brush.atlas_name[0] != 0) ? brush.atlas_name : "Select an atlas";
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##palette-atlas", preview)) {
            const u32 count = eng_asset_runtime_get_count();
            for (u32 i = 0; i < count; ++i) {
                asset_database_info info;
                if (!eng_asset_runtime_get_info(i, info))   continue;
                if (info.type != asset_type_e_sprite_atlas) continue;

                const bool is_selected = (strncmp(info.name, brush.atlas_name, MAP_BUILDER_NAME_SIZE) == 0);
                if (ImGui::Selectable(info.name, is_selected)) {
                    map_builder_select_atlas(info.name);
                }
            }
            ImGui::EndCombo();
        }
    }

    // loading an atlas that's already loaded gives back the one that's there
    IFB_INLINE void
    map_builder_select_atlas(
        const char* atlas_name) {

        auto& brush = _map_builder.brush;

        const hnd_sprite_atlas h_atlas = eng_sprite_atlas_load(atlas_name);
        if (h_atlas == INVALID_HANDLE) {
            snprintf(_map_builder.status, sizeof(_map_builder.status), "Couldn't load the atlas %s", atlas_name);
            return;
        }

        brush.h_atlas = h_atlas;
        strncpy_s(brush.atlas_name, atlas_name, MAP_BUILDER_NAME_SIZE - 1);
        brush.i_sprite       = INVALID_INDEX;
        brush.sprite_name[0] = 0;
    }

    IFB_INLINE void
    map_builder_load(
        const char* asset_name) {

        auto& state = _map_builder;

        map_builder_unload();

        state.h_map = eng_map_load_as(asset_name, MAP_BUILDER_MAP_NAME);
        if (state.h_map == INVALID_HANDLE) {
            snprintf(state.status, sizeof(state.status), "Couldn't load %s as a map", asset_name);
            return;
        }

        state.status[0]    = 0;
        state.save_path[0] = 0;
        state.is_modified  = false;

        // the builder's copy is named MAP-BUILDER, so the file's own name is read from the asset
        if (!eng_map_asset_get_name(asset_name, state.map_name, MAP_BUILDER_MAP_NAME_SIZE)) {
            state.map_name[0] = 0;
        }
        (void)eng_map_get_dimensions(state.h_map, state.rows, state.cols);
        map_builder_frame_map();
    }

    // writes the map to its file, a map file like the one it was loaded from, see docs/map-format.md
    // the game reads maps from the asset database, so the database has to be rebuilt to pick it up
    IFB_INLINE void
    map_builder_save(
        const bool is_save_as) {

        auto& state = _map_builder;
        if (state.h_map == INVALID_HANDLE) return;

        if (state.map_name[0] == 0) {
            snprintf(state.status, sizeof(state.status), "Give the map a name before saving it");
            return;
        }

        // ask where to save the first time, or when saving somewhere else
        if (is_save_as || state.save_path[0] == 0) {
            const char* path = eng_file_dialog_save(_map_builder_filter_type, _map_builder_filter_desc);
            if (path == NULL || path[0] == 0) return;

            char new_path[MAP_BUILDER_PATH_SIZE];
            strncpy_s(new_path, path, MAP_BUILDER_PATH_SIZE - 1);

            // the dialog doesn't add the extension
            const size_t length = strlen(new_path);
            const bool   has_ext = (length >= 5 && _stricmp(new_path + length - 5, ".json") == 0);
            if (!has_ext) {
                strncat_s(new_path, MAP_BUILDER_PATH_SIZE, ".json", MAP_BUILDER_PATH_SIZE - length - 1);
            }
            strncpy_s(state.save_path, new_path, MAP_BUILDER_PATH_SIZE - 1);
        }

        if (!eng_map_save(state.h_map, state.map_name, state.save_path)) {
            snprintf(state.status, sizeof(state.status), "Couldn't save the map to %s", state.save_path);
            return;
        }

        state.is_modified = false;
        snprintf(state.status, sizeof(state.status), "Saved to %s, rebuild the asset database to load it in the game", state.save_path);
    }

    IFB_INLINE void
    map_builder_unload(
        void) {

        auto& state = _map_builder;

        if (state.h_map != INVALID_HANDLE) {
            eng_map_destroy(state.h_map);
        }
        state.h_map          = INVALID_HANDLE;
        state.rows           = 0;
        state.cols           = 0;
        state.is_modified    = false;
        state.hover.is_valid = false;
        state.selected_chunk    = INVALID_INDEX;
        state.is_dragging_chunk = false;
        state.is_dragging_wall  = false;
    }

    // look down at the middle of the chunks, far enough back to see all of them
    IFB_INLINE void
    map_builder_frame_map(
        void) {

        auto& state = _map_builder;

        vec3 origin;
        if (!map_builder_get_map_origin(origin)) return;

        // the chunks' area, the max is one past the last tile, or the whole map if there are no chunks
        u32 row_min = 0;
        u32 col_min = 0;
        u32 row_max = state.rows;
        u32 col_max = state.cols;
        const bool has_chunks = eng_map_get_navigable_bounds(state.h_map, row_min, col_min, row_max, col_max);
        if (!has_chunks || row_max <= row_min || col_max <= col_min) {
            row_min = 0;
            col_min = 0;
            row_max = state.rows;
            col_max = state.cols;
        }

        const f32 tile     = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const f32 width    = (f32)(col_max - col_min) * tile;
        const f32 depth    = (f32)(row_max - row_min) * tile;
        const f32 extent   = (width > depth) ? width : depth;
        const f32 center_x = origin.x + ((f32)col_min * tile) + (width * 0.5f);
        const f32 center_z = origin.z + ((f32)row_min * tile) + (depth * 0.5f);

        state.camera.target   = { center_x, origin.y, center_z };
        state.camera.yaw      = 0.0f;
        state.camera.pitch    = 0.6f; // high enough to see the whole floor at once
        state.camera.distance = extent * 1.6f;
        if (state.camera.distance < MAP_BUILDER_DISTANCE_MIN) state.camera.distance = MAP_BUILDER_DISTANCE_MIN;
        if (state.camera.distance > MAP_BUILDER_DISTANCE_MAX) state.camera.distance = MAP_BUILDER_DISTANCE_MAX;
    }

    // the game's angle and distance, around what the camera is looking at now
    IFB_INLINE void
    map_builder_game_view(
        void) {

        auto& state = _map_builder;

        state.camera.yaw      = 0.0f;
        state.camera.pitch    = MAP_BUILDER_GAME_PITCH;
        state.camera.distance = MAP_BUILDER_GAME_DISTANCE;
    }

    IFB_INLINE void
    map_builder_apply_camera(
        void) {

        const auto& cam = _map_builder.camera;

        const f32 cos_pitch = cosf(cam.pitch);
        const vec3 offset = {
            sinf(cam.yaw) * cos_pitch * cam.distance,
            sinf(cam.pitch)           * cam.distance,
            cosf(cam.yaw) * cos_pitch * cam.distance
        };
        const vec3 origin = { cam.target.x + offset.x, cam.target.y + offset.y, cam.target.z + offset.z };

        eng_map_view_set_camera(origin, cam.target);
    }

    IFB_INLINE void
    map_builder_viewport(
        void) {

        auto& state = _map_builder;

        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x < 16.0f || size.y < 16.0f) return;
        size.x = floorf(size.x);
        size.y = floorf(size.y);

        // the button takes the mouse, so dragging in the view doesn't move the window
        const ImVec2 image_min = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(
            "##map-view",
            size,
            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        const bool is_hovered = ImGui::IsItemHovered();
        const bool is_active  = ImGui::IsItemActive();

        map_builder_update_camera (image_min, size, is_active, is_hovered);
        map_builder_apply_camera  ();
        map_builder_update_hover  (image_min, size, is_hovered || is_active);

        // painting follows the mouse while the left button is held in the view
        if (state.tool == map_builder_tool_e_brush && is_active && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            map_builder_paint();
        }

        if (state.tool == map_builder_tool_e_chunk) {
            map_builder_chunk_tool(is_active);

            const bool is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
            if (is_focused && !ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                map_builder_delete_selected_chunk();
            }
        }
        else {
            state.is_dragging_chunk = false;
        }

        if (state.tool == map_builder_tool_e_wall) {
            map_builder_wall_tool(is_active);
        }
        else {
            state.is_dragging_wall = false;
        }

        // the view draws at the end of this frame, so the image shows last frame's render
        eng_map_view_request(state.h_map, (u32)size.x, (u32)size.y);

        ImDrawList*       draw_list = ImGui::GetWindowDrawList();
        const ImVec2      image_max = ImVec2(image_min.x + size.x, image_min.y + size.y);
        const ImTextureID texture   = (ImTextureID)(intptr_t)eng_map_view_get_texture();
        draw_list->AddImage(texture, image_min, image_max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        if (state.h_map == INVALID_HANDLE) {
            const char*  text      = "Pick a map asset and load it";
            const ImVec2 text_size = ImGui::CalcTextSize(text);
            draw_list->AddText(
                ImVec2(image_min.x + (size.x - text_size.x) * 0.5f, image_min.y + (size.y - text_size.y) * 0.5f),
                ImGui::GetColorU32(ImGuiCol_TextDisabled),
                text);
            return;
        }

        // the wall tool outlines edges instead of tiles
        if (state.tool == map_builder_tool_e_wall) {
            map_builder_draw_walls(draw_list, image_min, size);
            return;
        }

        if (state.tool == map_builder_tool_e_chunk) {
            map_builder_draw_chunks(draw_list, image_min, size);
        }
        map_builder_draw_hover(draw_list, image_min, size);
    }

    IFB_INLINE void
    map_builder_update_camera(
        const ImVec2& image_min,
        const ImVec2& size,
        const bool    is_active,
        const bool    is_hovered) {

        const auto& state = _map_builder;
        auto&       cam   = _map_builder.camera;
        const auto& io    = ImGui::GetIO();

        // zoom towards the target
        if (is_hovered && io.MouseWheel != 0.0f) {
            cam.distance *= powf(MAP_BUILDER_ZOOM_STEP, io.MouseWheel);
            if (cam.distance < MAP_BUILDER_DISTANCE_MIN) cam.distance = MAP_BUILDER_DISTANCE_MIN;
            if (cam.distance > MAP_BUILDER_DISTANCE_MAX) cam.distance = MAP_BUILDER_DISTANCE_MAX;
        }

        if (!is_active) return;

        const f32 dx = io.MouseDelta.x;
        const f32 dy = io.MouseDelta.y;
        if (dx == 0.0f && dy == 0.0f) return;

        // the left button pans with the move tool, the middle button always pans
        const bool is_panning = (
            ImGui::IsMouseDown(ImGuiMouseButton_Middle) ||
            (state.tool == map_builder_tool_e_move && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        );

        // pan along the ground, keeping the point that was under the mouse under it
        if (is_panning) {

            const ImVec2 mouse_now  = io.MousePos;
            const ImVec2 mouse_prev = ImVec2(mouse_now.x - dx, mouse_now.y - dy);

            f32 x_now,  z_now;
            f32 x_prev, z_prev;
            const bool did_hit = (
                map_builder_screen_to_floor(mouse_now,  image_min, size, cam.target.y, x_now,  z_now) &&
                map_builder_screen_to_floor(mouse_prev, image_min, size, cam.target.y, x_prev, z_prev)
            );

            // the ground can be too far off to grab, like near the horizon
            const f32 move_x = x_prev - x_now;
            const f32 move_z = z_prev - z_now;
            const f32 move_limit = cam.distance;
            if (did_hit && fabsf(move_x) < move_limit && fabsf(move_z) < move_limit) {
                cam.target.x += move_x;
                cam.target.z += move_z;
            }
        }

        // orbit around the target
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            cam.yaw   -= dx * MAP_BUILDER_ORBIT_SPEED;
            cam.pitch += dy * MAP_BUILDER_ORBIT_SPEED;
            if (cam.pitch < MAP_BUILDER_PITCH_MIN) cam.pitch = MAP_BUILDER_PITCH_MIN;
            if (cam.pitch > MAP_BUILDER_PITCH_MAX) cam.pitch = MAP_BUILDER_PITCH_MAX;
        }
    }

    // finds the floor tile under the mouse
    IFB_INLINE void
    map_builder_update_hover(
        const ImVec2& image_min,
        const ImVec2& size,
        const bool    is_hovered) {

        auto& state = _map_builder;
        state.hover.is_valid    = false;
        state.hover.is_on_chunk = false;

        if (!is_hovered || state.h_map == INVALID_HANDLE) return;

        vec3 map_origin;
        if (!map_builder_get_map_origin(map_origin)) return;

        const ImVec2 mouse = ImGui::GetIO().MousePos;
        if (mouse.x < image_min.x || mouse.y < image_min.y || mouse.x > image_min.x + size.x || mouse.y > image_min.y + size.y) return;

        f32 hit_x;
        f32 hit_z;
        if (!map_builder_screen_to_floor(mouse, image_min, size, map_origin.y, hit_x, hit_z)) return;

        const f32 tile = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const s32 col  = (s32)floorf((hit_x - map_origin.x) / tile);
        const s32 row  = (s32)floorf((hit_z - map_origin.z) / tile);
        if (row < 0 || col < 0 || row >= (s32)state.rows || col >= (s32)state.cols) return;

        state.hover.is_valid    = true;
        state.hover.row         = row;
        state.hover.col         = col;
        state.hover.floor_row   = (hit_z - map_origin.z) / tile;
        state.hover.floor_col   = (hit_x - map_origin.x) / tile;
        state.hover.is_on_chunk = eng_map_get_tile_info(state.h_map, (u32)row, (u32)col, state.hover.tile);
    }

    // paints the hovered tile with the brush, if it's on a chunk and doesn't already have that sprite
    IFB_INLINE void
    map_builder_paint(
        void) {

        auto&       state = _map_builder;
        const auto& brush = state.brush;
        const auto& hover = state.hover;

        if (!hover.is_valid || !hover.is_on_chunk) return;

        const u32 row = (u32)hover.row;
        const u32 col = (u32)hover.col;

        // clearing
        if (brush.i_sprite == INVALID_INDEX) {
            if (!hover.tile.is_textured) return;
            if (eng_map_clear_tile_sprite(state.h_map, row, col)) {
                state.is_modified = true;
                state.status[0]   = 0;
            }
            return;
        }

        // painting a sprite, unless it's already there
        if (brush.h_atlas == INVALID_HANDLE) return;
        const bool is_same = (
            hover.tile.is_textured &&
            hover.tile.h_atlas  == brush.h_atlas &&
            hover.tile.i_sprite == brush.i_sprite
        );
        if (is_same) return;

        if (eng_map_set_tile_sprite(state.h_map, row, col, brush.atlas_name, brush.sprite_name)) {
            state.is_modified = true;
            state.status[0]   = 0;
        }
        else if (hover.tile.h_atlas != INVALID_HANDLE && hover.tile.h_atlas != brush.h_atlas) {
            snprintf(state.status, sizeof(state.status),
                "Chunk %u uses a different atlas, a chunk's sprites all come from one atlas", hover.tile.chunk_index);
        }
        else {
            snprintf(state.status, sizeof(state.status), "Couldn't paint row %u, col %u", row, col);
        }
    }

    //--------------------------------------------------------------------
    // CHUNK TOOL
    //--------------------------------------------------------------------

    // the color names, in map_color_e order, the same names as in a map file
    static const char* MAP_BUILDER_COLOR_NAMES[] = {
        "red_light",    "red_dark",
        "orange_light", "orange_dark",
        "yellow_light", "yellow_dark",
        "green_light",  "green_dark",
        "aqua_light",   "aqua_dark",
        "blue_light",   "blue_dark",
        "purple_light", "purple_dark",
        "gray_light",   "gray_dark",
        "none",
    };

    IFB_INLINE bool
    map_builder_color_combo(
        const char* label,
        u32&        color) {

        const u32   count   = sizeof(MAP_BUILDER_COLOR_NAMES) / sizeof(MAP_BUILDER_COLOR_NAMES[0]);
        const char* preview = (color < count) ? MAP_BUILDER_COLOR_NAMES[color] : "?";

        bool did_change = false;
        if (ImGui::BeginCombo(label, preview)) {
            for (u32 i = 0; i < count; ++i) {
                if (ImGui::Selectable(MAP_BUILDER_COLOR_NAMES[i], color == i)) {
                    color      = i;
                    did_change = true;
                }
            }
            ImGui::EndCombo();
        }
        return(did_change);
    }

    // the chunk tool's part of the palette panel
    IFB_INLINE void
    map_builder_chunk_panel(
        void) {

        auto& state = _map_builder;

        ImGui::SeparatorText("Chunks");
        if (state.h_map == INVALID_HANDLE) {
            ImGui::TextDisabled("No map loaded");
            return;
        }

        ImGui::TextWrapped("Drag on the map to add a chunk. Click a chunk to select it.");

        ImGui::TextUnformatted("New chunk color");
        ImGui::SetNextItemWidth(-FLT_MIN);
        map_builder_color_combo("##new-chunk-color", state.new_chunk_color);

        ImGui::TextDisabled("%u chunks", eng_map_get_chunk_count(state.h_map));

        map_chunk_info info;
        if (state.selected_chunk == INVALID_INDEX || !eng_map_chunk_get_info(state.h_map, state.selected_chunk, info)) {
            state.selected_chunk = INVALID_INDEX;
            return;
        }

        ImGui::SeparatorText("Selected");
        ImGui::Text("Chunk %u", state.selected_chunk);
        ImGui::Text("%u x %u at row %u, col %u", info.count_rows, info.count_cols, info.origin_row, info.origin_col);

        ImGui::TextUnformatted("Color");
        ImGui::SetNextItemWidth(-FLT_MIN);
        u32 color = info.color;
        if (map_builder_color_combo("##chunk-color", color)) {
            if (eng_map_chunk_set_color(state.h_map, state.selected_chunk, map_color_u32(color))) {
                state.is_modified = true;
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Tiles without a sprite show it, tiles with one are tinted by it");
        }

        // fill with whatever the brush paints, a sprite or no sprite
        const bool is_clearing = (state.brush.i_sprite == INVALID_INDEX);
        char fill_label[64];
        snprintf(fill_label, sizeof(fill_label), "Fill with %s", is_clearing ? "no sprite" : state.brush.sprite_name);
        if (ImGui::Button(fill_label, ImVec2(-FLT_MIN, 0.0f))) {
            if (is_clearing) {
                for (u32 r = 0; r < info.count_rows; ++r) {
                    for (u32 c = 0; c < info.count_cols; ++c) {
                        eng_map_clear_tile_sprite(state.h_map, info.origin_row + r, info.origin_col + c);
                    }
                }
                state.is_modified = true;
                state.status[0]   = 0;
            }
            else if (eng_map_chunk_set_sprite(state.h_map, state.selected_chunk, state.brush.atlas_name, state.brush.sprite_name)) {
                state.is_modified = true;
                state.status[0]   = 0;
            }
            else {
                snprintf(state.status, sizeof(state.status), "Couldn't fill chunk %u with %s", state.selected_chunk, state.brush.sprite_name);
            }
        }

        if (ImGui::Button("Delete chunk", ImVec2(-FLT_MIN, 0.0f))) {
            map_builder_delete_selected_chunk();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Also the Delete key. Its tiles and walls go with it.");
        }
    }

    IFB_INLINE void
    map_builder_delete_selected_chunk(
        void) {

        auto& state = _map_builder;
        if (state.h_map == INVALID_HANDLE || state.selected_chunk == INVALID_INDEX) return;

        if (eng_map_chunk_destroy(state.h_map, state.selected_chunk)) {
            snprintf(state.status, sizeof(state.status), "Deleted chunk %u, the chunks after it moved down one", state.selected_chunk);
            state.selected_chunk = INVALID_INDEX;
            state.is_modified    = true;
        }
    }

    // the tile rectangle from where the drag started to the tile under the mouse, the max is one past the last tile
    IFB_INLINE void
    map_builder_chunk_drag_rect(
        u32& row_min,
        u32& col_min,
        u32& row_max,
        u32& col_max) {

        const auto& state = _map_builder;
        const s32 r0 = state.drag_row;
        const s32 c0 = state.drag_col;
        const s32 r1 = state.drag_end_row;
        const s32 c1 = state.drag_end_col;

        row_min = (u32)((r0 < r1) ? r0 : r1);
        col_min = (u32)((c0 < c1) ? c0 : c1);
        row_max = (u32)((r0 < r1) ? r1 : r0) + 1;
        col_max = (u32)((c0 < c1) ? c1 : c0) + 1;
    }

    // a chunk fits if it's on the map and doesn't overlap another one, the same rules as eng_map_chunk_create
    IFB_INLINE bool
    map_builder_chunk_rect_is_free(
        const u32 row_min,
        const u32 col_min,
        const u32 row_max,
        const u32 col_max) {

        const auto& state = _map_builder;
        if (row_max > state.rows || col_max > state.cols) return(false);

        const u32 count = eng_map_get_chunk_count(state.h_map);
        for (u32 i = 0; i < count; ++i) {
            map_chunk_info info;
            if (!eng_map_chunk_get_info(state.h_map, i, info)) continue;
            const bool is_overlapping = (
                row_min < (info.origin_row + info.count_rows) && info.origin_row < row_max &&
                col_min < (info.origin_col + info.count_cols) && info.origin_col < col_max
            );
            if (is_overlapping) return(false);
        }
        return(true);
    }

    // dragging adds a chunk, clicking one tile selects the chunk on it or clears the selection
    IFB_INLINE void
    map_builder_chunk_tool(
        const bool is_active) {

        auto&       state = _map_builder;
        const auto& hover = state.hover;
        if (state.h_map == INVALID_HANDLE) return;

        if (is_active && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hover.is_valid) {
            state.is_dragging_chunk = true;
            state.drag_row          = hover.row;
            state.drag_col          = hover.col;
            state.drag_end_row      = hover.row;
            state.drag_end_col      = hover.col;
        }

        if (!state.is_dragging_chunk) return;

        // the end stays on the last tile the mouse was over, so leaving the map doesn't lose it
        if (hover.is_valid) {
            state.drag_end_row = hover.row;
            state.drag_end_col = hover.col;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) return;
        state.is_dragging_chunk = false;

        u32 row_min, col_min, row_max, col_max;
        map_builder_chunk_drag_rect(row_min, col_min, row_max, col_max);

        // one tile is a click
        if ((row_max - row_min) == 1 && (col_max - col_min) == 1) {
            map_tile_info tile;
            state.selected_chunk = eng_map_get_tile_info(state.h_map, row_min, col_min, tile)
                ? tile.chunk_index
                : INVALID_INDEX;
            return;
        }

        if (!map_builder_chunk_rect_is_free(row_min, col_min, row_max, col_max)) {
            snprintf(state.status, sizeof(state.status), "A chunk can't overlap another chunk");
            return;
        }

        const u32 chunk_index = eng_map_chunk_create(
            state.h_map,
            row_max - row_min,
            col_max - col_min,
            row_min,
            col_min,
            map_color_u32(state.new_chunk_color));

        if (chunk_index == INVALID_INDEX) {
            snprintf(state.status, sizeof(state.status), "Couldn't add a %u x %u chunk, the map may be out of room for tiles or walls",
                row_max - row_min, col_max - col_min);
            return;
        }

        state.selected_chunk = chunk_index;
        state.is_modified    = true;
        state.status[0]      = 0;
    }

    // outlines a rectangle of tiles on the floor, the max is one past the last tile
    IFB_INLINE void
    map_builder_draw_floor_rect(
        ImDrawList*   draw_list,
        const ImVec2& image_min,
        const ImVec2& size,
        const u32     row_min,
        const u32     col_min,
        const u32     row_max,
        const u32     col_max,
        const ImU32   fill,
        const ImU32   outline,
        const f32     thickness) {

        vec3 map_origin;
        if (!map_builder_get_map_origin(map_origin)) return;

        mat4 xform;
        eng_map_view_get_xform(xform);

        const f32 tile = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const f32 x0   = map_origin.x + ((f32)col_min * tile);
        const f32 x1   = map_origin.x + ((f32)col_max * tile);
        const f32 z0   = map_origin.z + ((f32)row_min * tile);
        const f32 z1   = map_origin.z + ((f32)row_max * tile);
        const vec3 corners[4] = {
            { x0, map_origin.y, z0 },
            { x1, map_origin.y, z0 },
            { x1, map_origin.y, z1 },
            { x0, map_origin.y, z1 },
        };

        ImVec2 points[4];
        for (u32 i = 0; i < 4; ++i) {
            if (!map_builder_world_to_screen(xform, corners[i], image_min, size, points[i])) return;
        }

        draw_list->PushClipRect(image_min, ImVec2(image_min.x + size.x, image_min.y + size.y), true);
        if (fill != 0) draw_list->AddQuadFilled(points[0], points[1], points[2], points[3], fill);
        draw_list->AddQuad(points[0], points[1], points[2], points[3], outline, thickness);
        draw_list->PopClipRect();
    }

    // every chunk's outline, the selected chunk, and the chunk being dragged out
    IFB_INLINE void
    map_builder_draw_chunks(
        ImDrawList*   draw_list,
        const ImVec2& image_min,
        const ImVec2& size) {

        const auto& state = _map_builder;

        const u32 count = eng_map_get_chunk_count(state.h_map);
        for (u32 i = 0; i < count; ++i) {
            map_chunk_info info;
            if (!eng_map_chunk_get_info(state.h_map, i, info)) continue;

            const bool  is_selected = (i == state.selected_chunk);
            const ImU32 outline     = is_selected ? IM_COL32(64, 220, 255, 255) : IM_COL32(255, 255, 255, 90);
            const ImU32 fill        = is_selected ? IM_COL32(64, 220, 255, 40)  : 0;
            map_builder_draw_floor_rect(
                draw_list, image_min, size,
                info.origin_row, info.origin_col,
                info.origin_row + info.count_rows, info.origin_col + info.count_cols,
                fill, outline, is_selected ? 2.5f : 1.0f);
        }

        if (!state.is_dragging_chunk) return;

        u32 row_min, col_min, row_max, col_max;
        map_builder_chunk_drag_rect(row_min, col_min, row_max, col_max);
        if ((row_max - row_min) == 1 && (col_max - col_min) == 1) return;

        const bool  is_free = map_builder_chunk_rect_is_free(row_min, col_min, row_max, col_max);
        const ImU32 fill    = is_free ? IM_COL32(64, 255, 120, 56)  : IM_COL32(255, 64, 64, 56);
        const ImU32 outline = is_free ? IM_COL32(64, 255, 120, 255) : IM_COL32(255, 64, 64, 255);
        map_builder_draw_floor_rect(draw_list, image_min, size, row_min, col_min, row_max, col_max, fill, outline, 2.0f);
    }

    //--------------------------------------------------------------------
    // WALL TOOL
    //--------------------------------------------------------------------

    // the wall tool's part of the palette panel
    IFB_INLINE void
    map_builder_wall_panel(
        void) {

        auto& state = _map_builder;

        ImGui::SeparatorText("Walls");
        if (state.h_map == INVALID_HANDLE) {
            ImGui::TextDisabled("No map loaded");
            return;
        }

        ImGui::TextWrapped("Drag along tile edges to place a straight run of walls. Hold Ctrl to erase.");

        int mode = state.is_wall_erase ? 1 : 0;
        ImGui::RadioButton("Place", &mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Erase", &mode, 1);
        state.is_wall_erase = (mode == 1);

        ImGui::TextUnformatted("Wall color");
        ImGui::SetNextItemWidth(-FLT_MIN);
        map_builder_color_combo("##wall-color", state.wall_color);

        // walls take the brush's sprite, like filling a chunk
        if (state.brush.i_sprite == INVALID_INDEX) ImGui::TextDisabled("Sprite: none");
        else                                       ImGui::Text("Sprite: %s", state.brush.sprite_name);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Walls take the brush's sprite. Pick No sprite below for solid color walls.");
        }
    }

    // the edge of the hovered tile closest to the mouse
    IFB_INLINE bool
    map_builder_hovered_edge(
        map_builder_edge& edge) {

        const auto& hover = _map_builder.hover;
        if (!hover.is_valid) return(false);

        // how far across the tile the mouse is, 0 at the min edge and 1 at the max edge
        const f32 across_col = hover.floor_col - (f32)hover.col;
        const f32 across_row = hover.floor_row - (f32)hover.row;

        // row walls are on the lines z = row, column walls on the lines x = col
        f32 nearest = across_row;
        edge.orientation = map_wall_orientation_e_row;
        edge.row         = hover.row;
        edge.col         = hover.col;

        if ((1.0f - across_row) < nearest) {
            nearest          = 1.0f - across_row;
            edge.orientation = map_wall_orientation_e_row;
            edge.row         = hover.row + 1;
            edge.col         = hover.col;
        }
        if (across_col < nearest) {
            nearest          = across_col;
            edge.orientation = map_wall_orientation_e_column;
            edge.row         = hover.row;
            edge.col         = hover.col;
        }
        if ((1.0f - across_col) < nearest) {
            edge.orientation = map_wall_orientation_e_column;
            edge.row         = hover.row;
            edge.col         = hover.col + 1;
        }
        return(true);
    }

    // the edge at a step along the drag's line, row walls step along columns and column walls along rows
    IFB_INLINE map_builder_edge
    map_builder_wall_drag_edge(
        const s32 step) {

        map_builder_edge edge = _map_builder.wall_start;
        if (edge.orientation == map_wall_orientation_e_row) edge.col = step;
        else                                                 edge.row = step;
        return(edge);
    }

    IFB_INLINE void
    map_builder_wall_drag_range(
        s32& step_min,
        s32& step_max) {

        const auto& state      = _map_builder;
        const s32   start_step = (state.wall_start.orientation == map_wall_orientation_e_row)
            ? state.wall_start.col
            : state.wall_start.row;

        step_min = (start_step < state.wall_end_step) ? start_step : state.wall_end_step;
        step_max = (start_step < state.wall_end_step) ? state.wall_end_step : start_step;
    }

    // a drag starts on the nearest edge, and stays on that edge's line, so it makes a straight run
    // the walls are placed or erased when the button is let go
    IFB_INLINE void
    map_builder_wall_tool(
        const bool is_active) {

        auto&       state = _map_builder;
        const auto& brush = state.brush;
        if (state.h_map == INVALID_HANDLE) return;

        if (is_active && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            map_builder_edge edge;
            if (map_builder_hovered_edge(edge)) {
                state.is_dragging_wall = true;
                state.is_drag_erasing  = state.is_wall_erase || ImGui::GetIO().KeyCtrl;
                state.wall_start       = edge;
                state.wall_end_step    = (edge.orientation == map_wall_orientation_e_row) ? edge.col : edge.row;
            }
        }

        if (!state.is_dragging_wall) return;

        // follow the mouse along the line, the end stays put while the mouse is off the map
        if (state.hover.is_valid) {
            const bool is_row = (state.wall_start.orientation == map_wall_orientation_e_row);
            const s32  last   = is_row ? ((s32)state.cols - 1) : ((s32)state.rows - 1);
            s32        step   = (s32)floorf(is_row ? state.hover.floor_col : state.hover.floor_row);
            if (step < 0)    step = 0;
            if (step > last) step = last;
            state.wall_end_step = step;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) return;
        state.is_dragging_wall = false;

        s32 step_min, step_max;
        map_builder_wall_drag_range(step_min, step_max);

        const bool has_sprite = (brush.i_sprite != INVALID_INDEX);
        u32 changed       = 0;
        u32 off_chunk     = 0;
        u32 sprite_failed = 0;
        for (s32 step = step_min; step <= step_max; ++step) {

            const map_builder_edge edge = map_builder_wall_drag_edge(step);
            const u32 row = (u32)edge.row;
            const u32 col = (u32)edge.col;

            // walls only go on chunk edges
            map_wall_info info;
            if (!eng_map_wall_get_info(state.h_map, row, col, edge.orientation, info)) {
                ++off_chunk;
                continue;
            }

            if (state.is_drag_erasing) {
                if (info.is_placed && eng_map_wall_clear(state.h_map, row, col, edge.orientation)) ++changed;
                continue;
            }

            if (!eng_map_wall_set(state.h_map, row, col, edge.orientation, map_color_u32(state.wall_color))) {
                ++off_chunk;
                continue;
            }
            ++changed;

            // a chunk's sprites all come from one atlas, so a sprite from another one doesn't go on
            if (has_sprite && !eng_map_wall_set_sprite(state.h_map, row, col, edge.orientation, brush.atlas_name, brush.sprite_name)) {
                ++sprite_failed;
            }
        }

        if (changed != 0) state.is_modified = true;

        u32 length = (u32)snprintf(state.status, sizeof(state.status), "%s %u wall%s",
            state.is_drag_erasing ? "Erased" : "Placed", changed, (changed == 1) ? "" : "s");
        if (off_chunk != 0 && length < sizeof(state.status)) {
            length += (u32)snprintf(state.status + length, sizeof(state.status) - length,
                ", %u edge%s not on a chunk", off_chunk, (off_chunk == 1) ? " is" : "s are");
        }
        if (sprite_failed != 0 && length < sizeof(state.status)) {
            snprintf(state.status + length, sizeof(state.status) - length,
                ", %u couldn't take %s because their chunk uses another atlas", sprite_failed, brush.sprite_name);
        }
    }

    // a wall's face standing on an edge, a tile tall like the walls the game draws
    IFB_INLINE void
    map_builder_draw_edge(
        ImDrawList*             draw_list,
        const ImVec2&           image_min,
        const ImVec2&           size,
        const map_builder_edge& edge,
        const ImU32             fill,
        const ImU32             outline) {

        vec3 map_origin;
        if (!map_builder_get_map_origin(map_origin)) return;

        mat4 xform;
        eng_map_view_get_xform(xform);

        const f32  tile   = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const bool is_row = (edge.orientation == map_wall_orientation_e_row);
        const f32  x0     = map_origin.x + ((f32)edge.col * tile);
        const f32  z0     = map_origin.z + ((f32)edge.row * tile);
        const f32  x1     = is_row ? (x0 + tile) : x0;
        const f32  z1     = is_row ? z0 : (z0 + tile);
        const f32  y0     = map_origin.y;
        const f32  y1     = map_origin.y + tile;
        const vec3 corners[4] = {
            { x0, y0, z0 },
            { x1, y0, z1 },
            { x1, y1, z1 },
            { x0, y1, z0 },
        };

        ImVec2 points[4];
        for (u32 i = 0; i < 4; ++i) {
            if (!map_builder_world_to_screen(xform, corners[i], image_min, size, points[i])) return;
        }

        draw_list->PushClipRect(image_min, ImVec2(image_min.x + size.x, image_min.y + size.y), true);
        if (fill != 0) draw_list->AddQuadFilled(points[0], points[1], points[2], points[3], fill);
        draw_list->AddQuad(points[0], points[1], points[2], points[3], outline, 2.0f);
        draw_list->PopClipRect();
    }

    // the run being dragged, or the edge under the mouse
    // green places, red erases or can't go there, gray edges aren't on a chunk
    IFB_INLINE void
    map_builder_draw_walls(
        ImDrawList*   draw_list,
        const ImVec2& image_min,
        const ImVec2& size) {

        const auto& state = _map_builder;

        const ImU32 place_fill    = IM_COL32(64, 255, 120, 70);
        const ImU32 place_outline = IM_COL32(64, 255, 120, 255);
        const ImU32 erase_fill    = IM_COL32(255, 64, 64, 70);
        const ImU32 erase_outline = IM_COL32(255, 64, 64, 255);
        const ImU32 off_outline   = IM_COL32(160, 160, 160, 200);

        if (state.is_dragging_wall) {
            s32 step_min, step_max;
            map_builder_wall_drag_range(step_min, step_max);
            for (s32 step = step_min; step <= step_max; ++step) {
                const map_builder_edge edge = map_builder_wall_drag_edge(step);
                map_wall_info info;
                if (!eng_map_wall_get_info(state.h_map, (u32)edge.row, (u32)edge.col, edge.orientation, info)) {
                    map_builder_draw_edge(draw_list, image_min, size, edge, 0, off_outline);
                }
                else if (state.is_drag_erasing) {
                    map_builder_draw_edge(draw_list, image_min, size, edge, erase_fill, erase_outline);
                }
                else {
                    map_builder_draw_edge(draw_list, image_min, size, edge, place_fill, place_outline);
                }
            }
            return;
        }

        map_builder_edge edge;
        if (!map_builder_hovered_edge(edge)) return;

        map_wall_info info;
        const bool is_on_chunk = eng_map_wall_get_info(state.h_map, (u32)edge.row, (u32)edge.col, edge.orientation, info);
        const bool is_erasing  = state.is_wall_erase || ImGui::GetIO().KeyCtrl;
        if (!is_on_chunk)    map_builder_draw_edge(draw_list, image_min, size, edge, erase_fill, erase_outline);
        else if (is_erasing) map_builder_draw_edge(draw_list, image_min, size, edge, erase_fill, erase_outline);
        else                 map_builder_draw_edge(draw_list, image_min, size, edge, IM_COL32(255, 220, 64, 60), IM_COL32(255, 220, 64, 255));
    }

    // the hovered edge and what's on it
    IFB_INLINE void
    map_builder_wall_text(
        void) {

        const auto& state = _map_builder;

        map_builder_edge edge;
        if (state.h_map == INVALID_HANDLE || !map_builder_hovered_edge(edge)) {
            ImGui::TextUnformatted("");
            return;
        }

        const char* orientation_name = (edge.orientation == map_wall_orientation_e_row) ? "row" : "column";

        map_wall_info info;
        if (!eng_map_wall_get_info(state.h_map, (u32)edge.row, (u32)edge.col, edge.orientation, info)) {
            ImGui::Text("   %s edge at row %d, col %d   no chunk", orientation_name, edge.row, edge.col);
            return;
        }

        if (!info.is_placed) {
            ImGui::Text("   %s edge at row %d, col %d   chunk %u   no wall", orientation_name, edge.row, edge.col, info.chunk_index);
            return;
        }

        const u32   color_count = sizeof(MAP_BUILDER_COLOR_NAMES) / sizeof(MAP_BUILDER_COLOR_NAMES[0]);
        const char* color_name  = (info.color < color_count) ? MAP_BUILDER_COLOR_NAMES[info.color] : "?";
        char sprite_name[MAP_BUILDER_NAME_SIZE] = "no sprite";
        if (info.i_sprite != INVALID_INDEX) {
            map_builder_get_sprite_name(info.h_atlas, info.i_sprite, sprite_name);
        }
        ImGui::Text("   %s wall at row %d, col %d   chunk %u   %s   %s",
            orientation_name, edge.row, edge.col, info.chunk_index, color_name, sprite_name);
    }

    // the hovered tile, its chunk, and what's on it
    IFB_INLINE void
    map_builder_hover_text(
        void) {

        const auto& hover = _map_builder.hover;
        if (!hover.is_valid) {
            ImGui::TextUnformatted("");
            return;
        }

        if (!hover.is_on_chunk) {
            ImGui::Text("   row %d, col %d   no chunk", hover.row, hover.col);
            return;
        }

        char sprite_name[MAP_BUILDER_NAME_SIZE] = "no sprite";
        if (hover.tile.is_textured) {
            map_builder_get_sprite_name(hover.tile.h_atlas, hover.tile.i_sprite, sprite_name);
        }
        ImGui::Text("   row %d, col %d   chunk %u   %s", hover.row, hover.col, hover.tile.chunk_index, sprite_name);
    }

    // outlines the hovered tile, by projecting its corners into the view
    // yellow on a chunk, red off of one, where it can't be painted
    IFB_INLINE void
    map_builder_draw_hover(
        ImDrawList*   draw_list,
        const ImVec2& image_min,
        const ImVec2& size) {

        const auto& state = _map_builder;
        if (!state.hover.is_valid) return;

        vec3 map_origin;
        if (!map_builder_get_map_origin(map_origin)) return;

        mat4 xform;
        eng_map_view_get_xform(xform);

        const f32 tile = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const f32 x0   = map_origin.x + ((f32)state.hover.col * tile);
        const f32 z0   = map_origin.z + ((f32)state.hover.row * tile);
        const vec3 corners[4] = {
            { x0,        map_origin.y, z0        },
            { x0 + tile, map_origin.y, z0        },
            { x0 + tile, map_origin.y, z0 + tile },
            { x0,        map_origin.y, z0 + tile },
        };

        ImVec2 points[4];
        for (u32 i = 0; i < 4; ++i) {
            if (!map_builder_world_to_screen(xform, corners[i], image_min, size, points[i])) return;
        }

        const ImU32 fill    = state.hover.is_on_chunk ? IM_COL32(255, 220, 64, 48)  : IM_COL32(255, 64, 64, 48);
        const ImU32 outline = state.hover.is_on_chunk ? IM_COL32(255, 220, 64, 255) : IM_COL32(255, 64, 64, 255);

        draw_list->PushClipRect(image_min, ImVec2(image_min.x + size.x, image_min.y + size.y), true);
        draw_list->AddQuadFilled (points[0], points[1], points[2], points[3], fill);
        draw_list->AddQuad       (points[0], points[1], points[2], points[3], outline, 2.0f);
        draw_list->PopClipRect();
    }

    // where tile (0, 0) starts in the world
    IFB_INLINE bool
    map_builder_get_map_origin(
        vec3& origin) {

        const auto& state = _map_builder;
        if (state.h_map == INVALID_HANDLE) return(false);

        cmpnt_map_coords coords = {};
        coords.h_map = state.h_map;

        cmpnt_position pos;
        if (!eng_map_get_pos_from_coords(coords, pos)) return(false);

        origin = { pos.x, pos.y, pos.z };
        return(true);
    }

    // sprite names fill all 16 characters of an asset name, so they're copied into a buffer with room for the terminator
    IFB_INLINE bool
    map_builder_get_sprite_name(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        char*                  name) {

        name[0] = 0;

        sprite_info info;
        if (h_atlas == INVALID_HANDLE || !eng_sprite_atlas_get_sprite_info(h_atlas, i_sprite, info)) return(false);

        memcpy(name, info.name.cstr, config::asset_name_size);
        name[config::asset_name_size] = 0;
        return(true);
    }

    // the point on the plane y = plane_y that's drawn at a point in the view, the inverse of map_builder_world_to_screen
    // a point (x, plane_y, z) lands on ndc (nx, ny) when clip_x = nx * clip_w and clip_y = ny * clip_w,
    // which is two linear equations in x and z, so this inverts whatever projection the view uses
    // false if the point is on or past the horizon, or behind the camera
    IFB_INLINE bool
    map_builder_screen_to_floor(
        const ImVec2& screen,
        const ImVec2& image_min,
        const ImVec2& size,
        const f32     plane_y,
        f32&          x,
        f32&          z) {

        mat4 m;
        eng_map_view_get_xform(m);

        const f32 nx = (((screen.x - image_min.x) / size.x) * 2.0f) - 1.0f;
        const f32 ny = 1.0f - (((screen.y - image_min.y) / size.y) * 2.0f);

        // (row 0 - nx * row 3) . (x, y, z, 1) = 0, and the same for row 1 and ny
        const f32 a0 = m.r0c0 - (nx * m.r3c0);
        const f32 c0 = m.r0c2 - (nx * m.r3c2);
        const f32 d0 = ((m.r0c1 - (nx * m.r3c1)) * plane_y) + (m.r0c3 - (nx * m.r3c3));
        const f32 a1 = m.r1c0 - (ny * m.r3c0);
        const f32 c1 = m.r1c2 - (ny * m.r3c2);
        const f32 d1 = ((m.r1c1 - (ny * m.r3c1)) * plane_y) + (m.r1c3 - (ny * m.r3c3));

        const f32 det = (a0 * c1) - (c0 * a1);
        if (fabsf(det) < 0.000001f) return(false);

        x = ((c0 * d1) - (d0 * c1)) / det;
        z = ((d0 * a1) - (a0 * d1)) / det;

        // the point has to be in front of the camera
        const f32 clip_w = (m.r3c0 * x) + (m.r3c1 * plane_y) + (m.r3c2 * z) + m.r3c3;
        return(clip_w > 0.0001f);
    }

    IFB_INLINE bool
    map_builder_world_to_screen(
        const mat4&   xform,
        const vec3&   world,
        const ImVec2& image_min,
        const ImVec2& size,
        ImVec2&       screen) {

        const f32 clip_x = (xform.r0c0 * world.x) + (xform.r0c1 * world.y) + (xform.r0c2 * world.z) + xform.r0c3;
        const f32 clip_y = (xform.r1c0 * world.x) + (xform.r1c1 * world.y) + (xform.r1c2 * world.z) + xform.r1c3;
        const f32 clip_w = (xform.r3c0 * world.x) + (xform.r3c1 * world.y) + (xform.r3c2 * world.z) + xform.r3c3;

        // behind the camera
        if (clip_w <= 0.0001f) return(false);

        const f32 ndc_x = clip_x / clip_w;
        const f32 ndc_y = clip_y / clip_w;
        screen.x = image_min.x + (((ndc_x + 1.0f) * 0.5f) * size.x);
        screen.y = image_min.y + (((1.0f - ndc_y) * 0.5f) * size.y);
        return(true);
    }
};
