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

    static constexpr u32 MAP_BUILDER_NAME_SIZE     = config::asset_name_size + 1;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // an orbit camera around a target, turned into an origin and target for the map view
    struct map_builder_camera {
        vec3 target;
        f32  yaw;      // 0 looks down -z, like the game
        f32  pitch;    // up from the ground
        f32  distance;
    };

    struct map_builder_hover {
        bool is_valid;
        s32  row;
        s32  col;
    };

    struct map_builder_state {
        hnd_map            h_map = {INVALID_HANDLE};
        char               asset_name [MAP_BUILDER_NAME_SIZE];
        char               status     [128];
        map_builder_camera camera;
        map_builder_hover  hover;
        u32                rows;
        u32                cols;
        bool               show_grid;
    } static _map_builder;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void map_builder_toolbar          (void);
    IFB_INLINE void map_builder_load             (const char* asset_name);
    IFB_INLINE void map_builder_unload           (void);
    IFB_INLINE void map_builder_frame_map        (void);
    IFB_INLINE void map_builder_game_view        (void);
    IFB_INLINE void map_builder_apply_camera     (void);
    IFB_INLINE void map_builder_viewport         (void);
    IFB_INLINE void map_builder_update_camera    (const ImVec2& size, const bool is_active, const bool is_hovered);
    IFB_INLINE void map_builder_update_hover     (const ImVec2& image_min, const ImVec2& size, const bool is_hovered);
    IFB_INLINE void map_builder_draw_hover       (ImDrawList* draw_list, const ImVec2& image_min, const ImVec2& size);
    IFB_INLINE bool map_builder_get_map_origin   (vec3& origin);
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
            is_initialized       = true;
            state.camera.target  = { 0.0f, 0.2f, 0.1f };
            state.camera.yaw     = 0.0f;
            state.camera.pitch   = MAP_BUILDER_GAME_PITCH;
            state.camera.distance = MAP_BUILDER_GAME_DISTANCE;
            state.show_grid      = false;
            state.status[0]      = 0;
            state.asset_name[0]  = 0;
        }

        ImGui::SetNextWindowSize(ImVec2(960.0f, 640.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(_gui_str->menu_engine_menu_tools_item_map_builder, &is_open)) {
            map_builder_toolbar  ();
            map_builder_viewport ();
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

        ImGui::SameLine();
        ImGui::BeginDisabled(state.asset_name[0] == 0);
        if (ImGui::Button("Load")) {
            map_builder_load(state.asset_name);
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

        // what's loaded, what's under the mouse, and how to move around
        if (state.status[0] != 0) {
            ImGui::TextUnformatted(state.status);
        }
        else if (state.h_map == INVALID_HANDLE) {
            ImGui::TextDisabled("No map loaded");
        }
        else {
            ImGui::Text("%s  %u x %u", state.asset_name, state.rows, state.cols);
        }

        ImGui::SameLine();
        if (state.hover.is_valid) {
            ImGui::Text("   row %d, col %d", state.hover.row, state.hover.col);
        }

        ImGui::SameLine();
        ImGui::TextDisabled("   left drag: pan   right drag: orbit   wheel: zoom");
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

        state.status[0] = 0;
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

        const f32 tile   = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const f32 width  = (f32)(col_max - col_min) * tile;
        const f32 depth  = (f32)(row_max - row_min) * tile;
        const f32 extent = (width > depth) ? width : depth;
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
        ImGui::InvisibleButton("##map-view", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered();
        const bool is_active  = ImGui::IsItemActive();

        map_builder_update_camera (size, is_active, is_hovered);
        map_builder_apply_camera  ();
        map_builder_update_hover  (image_min, size, is_hovered);

        // the view draws at the end of this frame, so the image shows last frame's render
        eng_map_view_request(state.h_map, (u32)size.x, (u32)size.y);

        ImDrawList*       draw_list  = ImGui::GetWindowDrawList();
        const ImVec2      image_max  = ImVec2(image_min.x + size.x, image_min.y + size.y);
        const ImTextureID texture    = (ImTextureID)(intptr_t)eng_map_view_get_texture();
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
        const ImVec2& size,
        const bool    is_active,
        const bool    is_hovered) {

        auto&       cam = _map_builder.camera;
        const auto& io  = ImGui::GetIO();

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

        // pan along the ground, so the map stays under the mouse at the target's depth
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

            mat4 xform;
            eng_map_view_get_xform(xform);
            const f32 tan_half_fov  = (xform.r1c1 != 0.0f) ? (1.0f / xform.r1c1) : 0.268f;
            const f32 world_per_px  = (2.0f * cam.distance * tan_half_fov) / size.y;

            // right and forward on the ground, from the yaw
            const f32 right_x   =  cosf(cam.yaw);
            const f32 right_z   = -sinf(cam.yaw);
            const f32 forward_x = -sinf(cam.yaw);
            const f32 forward_z = -cosf(cam.yaw);

            // looking down at a slant, the ground is stretched along forward
            const f32 sin_pitch = sinf(cam.pitch);
            const f32 forward_scale = (sin_pitch > 0.05f) ? (1.0f / sin_pitch) : 20.0f;

            cam.target.x += (-dx * right_x * world_per_px) + (dy * forward_x * world_per_px * forward_scale);
            cam.target.z += (-dx * right_z * world_per_px) + (dy * forward_z * world_per_px * forward_scale);
        }

        // orbit around the target
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            cam.yaw   -= dx * MAP_BUILDER_ORBIT_SPEED;
            cam.pitch += dy * MAP_BUILDER_ORBIT_SPEED;
            if (cam.pitch < MAP_BUILDER_PITCH_MIN) cam.pitch = MAP_BUILDER_PITCH_MIN;
            if (cam.pitch > MAP_BUILDER_PITCH_MAX) cam.pitch = MAP_BUILDER_PITCH_MAX;
        }
    }

    // casts a ray from the camera through the mouse onto the floor, and finds the tile it hits
    IFB_INLINE void
    map_builder_update_hover(
        const ImVec2& image_min,
        const ImVec2& size,
        const bool    is_hovered) {

        auto& state = _map_builder;
        state.hover.is_valid = false;

        if (!is_hovered || state.h_map == INVALID_HANDLE) return;

        vec3 map_origin;
        if (!map_builder_get_map_origin(map_origin)) return;

        // the mouse in normalized device coordinates, y up
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const f32    ndc_x = (((mouse.x - image_min.x) / size.x) * 2.0f) - 1.0f;
        const f32    ndc_y = 1.0f - (((mouse.y - image_min.y) / size.y) * 2.0f);

        // the projection has f = 1 / tan(fov / 2) at r1c1 and f / aspect at r0c0
        mat4 xform;
        eng_map_view_get_xform(xform);
        if (xform.r0c0 == 0.0f || xform.r1c1 == 0.0f) return;
        const f32 tan_half_fov_y = 1.0f / xform.r1c1;
        const f32 tan_half_fov_x = 1.0f / xform.r0c0;

        // the camera's axes
        vec3 origin;
        vec3 target;
        eng_map_view_get_camera(origin, target);

        vec3 forward = { target.x - origin.x, target.y - origin.y, target.z - origin.z };
        const f32 forward_len = sqrtf((forward.x * forward.x) + (forward.y * forward.y) + (forward.z * forward.z));
        if (forward_len == 0.0f) return;
        forward = { forward.x / forward_len, forward.y / forward_len, forward.z / forward_len };

        // right = forward x up, up = right x forward
        vec3 right = { -forward.z, 0.0f, forward.x };
        const f32 right_len = sqrtf((right.x * right.x) + (right.z * right.z));
        if (right_len == 0.0f) return;
        right = { right.x / right_len, 0.0f, right.z / right_len };
        const vec3 up = {
            (right.y * forward.z) - (right.z * forward.y),
            (right.z * forward.x) - (right.x * forward.z),
            (right.x * forward.y) - (right.y * forward.x)
        };

        const vec3 ray = {
            forward.x + (right.x * ndc_x * tan_half_fov_x) + (up.x * ndc_y * tan_half_fov_y),
            forward.y + (right.y * ndc_x * tan_half_fov_x) + (up.y * ndc_y * tan_half_fov_y),
            forward.z + (right.z * ndc_x * tan_half_fov_x) + (up.z * ndc_y * tan_half_fov_y)
        };

        // the floor is the plane y = the map's origin, and the ray has to go down to reach it
        if (ray.y >= -0.0001f) return;
        const f32 t = (map_origin.y - origin.y) / ray.y;
        if (t <= 0.0f) return;

        const f32 hit_x = origin.x + (ray.x * t);
        const f32 hit_z = origin.z + (ray.z * t);

        const f32 tile = IFB_CONFIG_MAP_TILE_UNIT_SIZE;
        const s32 col  = (s32)floorf((hit_x - map_origin.x) / tile);
        const s32 row  = (s32)floorf((hit_z - map_origin.z) / tile);
        if (row < 0 || col < 0 || row >= (s32)state.rows || col >= (s32)state.cols) return;

        state.hover.is_valid = true;
        state.hover.row      = row;
        state.hover.col      = col;
    }

    // outlines the hovered tile, by projecting its corners into the view
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

        draw_list->PushClipRect(image_min, ImVec2(image_min.x + size.x, image_min.y + size.y), true);
        draw_list->AddQuadFilled (points[0], points[1], points[2], points[3], IM_COL32(255, 220, 64, 48));
        draw_list->AddQuad       (points[0], points[1], points[2], points[3], IM_COL32(255, 220, 64, 255), 2.0f);
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
