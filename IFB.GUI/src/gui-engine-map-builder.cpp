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

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    enum map_builder_tool_e {
        map_builder_tool_e_move  = 0, // left drag pans
        map_builder_tool_e_brush = 1, // left click and drag paints tiles
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
        map_tile_info tile;
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
        char               status     [128];
        map_builder_camera camera;
        map_builder_hover  hover;
        map_builder_brush  brush;
        map_builder_tool_e tool;
        u32                rows;
        u32                cols;
        bool               show_grid;
        bool               is_modified;
    } static _map_builder;


    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void map_builder_toolbar          (void);
    IFB_INLINE void map_builder_palette          (void);
    IFB_INLINE void map_builder_palette_atlas    (void);
    IFB_INLINE void map_builder_select_atlas     (const char* atlas_name);
    IFB_INLINE void map_builder_load             (const char* asset_name);
    IFB_INLINE void map_builder_unload           (void);
    IFB_INLINE void map_builder_frame_map        (void);
    IFB_INLINE void map_builder_game_view        (void);
    IFB_INLINE void map_builder_apply_camera     (void);
    IFB_INLINE void map_builder_viewport         (void);
    IFB_INLINE void map_builder_update_camera    (const ImVec2& image_min, const ImVec2& size, const bool is_active, const bool is_hovered);
    IFB_INLINE void map_builder_update_hover     (const ImVec2& image_min, const ImVec2& size, const bool is_hovered);
    IFB_INLINE void map_builder_paint            (void);
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
        map_builder_hover_text();

        ImGui::SameLine();
        if (state.tool == map_builder_tool_e_brush) {
            ImGui::TextDisabled("   left: paint   middle drag: pan   right drag: orbit   wheel: zoom");
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
            state.tool           = map_builder_tool_e_brush;
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
                state.tool = map_builder_tool_e_brush;
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

        state.status[0]   = 0;
        state.is_modified = false;
        (void)eng_map_get_dimensions(state.h_map, state.rows, state.cols);
        map_builder_frame_map();
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
