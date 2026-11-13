#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include <math.h>

namespace ifb {

    // shows the tile under the mouse in a tooltip, for placing npc spawns
    // the chunk and chunk x / y are what game_npc_config takes
    IFB_INTERNAL void
    gui_game_tile_coords(
        const bool          is_enabled,
        const game_context* game_ctx) {

        if (!is_enabled)                                              return;
        if (game_ctx == NULL || game_ctx->map == NULL)                return;
        if (game_ctx->map->eng_hnd == INVALID_HANDLE)                 return;

        // only over the game itself, not over a gui window
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))      return;
        if (ImGui::IsAnyItemHovered())                                return;
        if (!ImGui::IsMousePosValid())                                return;

        const hnd_map h_map = game_ctx->map->eng_hnd;

        // the floor is at the map origin's height
        cmpnt_map_coords origin_coords = {};
        origin_coords.h_map = h_map;
        cmpnt_position origin_pos;
        if (!eng_map_get_pos_from_coords(origin_coords, origin_pos))  return;

        // the game draws over the whole window
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2         mouse    = ImGui::GetMousePos();

        mat4 view_proj;
        eng_camera_get_view_proj(view_proj);

        const plane floor_plane = { { 0.0f, 1.0f, 0.0f }, origin_pos.y };

        vec3 hit;
        const bool did_hit = xform_viewport_to_plane(
            view_proj,
            { mouse.x,          mouse.y          },
            { viewport->Pos.x,  viewport->Pos.y  },
            { viewport->Size.x, viewport->Size.y },
            floor_plane,
            hit);
        if (!did_hit) return;

        cmpnt_map_coords coords = {};
        coords.h_map = h_map;
        if (!eng_map_get_coords_from_pos(hit, coords)) return;

        u32 map_rows = 0;
        u32 map_cols = 0;
        if (!eng_map_get_dimensions(h_map, map_rows, map_cols)) return;

        const f32  col_f     = floorf(coords.col_x);
        const f32  row_f     = floorf(coords.row_z);
        const bool is_on_map = (col_f >= 0.0f && row_f >= 0.0f && col_f < (f32)map_cols && row_f < (f32)map_rows);

        ImGui::BeginTooltip();

        if (!is_on_map) {
            ImGui::TextDisabled("Off the map");
            ImGui::EndTooltip();
            return;
        }

        const u32 col = (u32)col_f;
        const u32 row = (u32)row_f;

        ImGui::Text("Map   col %u, row %u", col, row);

        // find the chunk the tile is on, npcs spawn relative to its origin
        bool      is_on_chunk = false;
        const u32 chunk_count = eng_map_get_chunk_count(h_map);
        for (u32 i_chunk = 0; i_chunk < chunk_count; ++i_chunk) {

            u32 chunk_rows = 0;
            u32 chunk_cols = 0;
            u32 origin_row = 0;
            u32 origin_col = 0;
            if (!eng_map_chunk_get_dimensions(h_map, i_chunk, chunk_rows, chunk_cols, origin_row, origin_col)) continue;

            const bool is_in_chunk =
                col >= origin_col && col < origin_col + chunk_cols &&
                row >= origin_row && row < origin_row + chunk_rows;
            if (!is_in_chunk) continue;

            ImGui::Text("Chunk %u", i_chunk);
            ImGui::Text("      chunk_x %u, chunk_y %u", col - origin_col, row - origin_row);
            is_on_chunk = true;
            break;
        }

        if (!is_on_chunk) {
            ImGui::TextDisabled("Not on a chunk");
        }

        ImGui::EndTooltip();
    }
};
