#pragma once

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "eng-map.hpp"
#include "map.cpp"
#include <string.h>

namespace ifb {

    IFB_INTERNAL u32
    map_chunk_create(
        const hnd_map    map_hnd,
        const u32           count_rows,
        const u32           count_cols,
        const u32           origin_row,
        const u32           origin_col,
        const map_color_u32 base_color) {

        const auto& cfg = config_instance();

        assert(map_hnd    != INVALID_HANDLE);
        assert(count_rows != 0);
        assert(count_cols != 0);
   
        // get the map index
        // if it doesn't exist, we're done
        const u32 map_index = map_lookup_index(map_hnd);    
        if (map_index == INVALID_INDEX) {
            return(INVALID_INDEX);
        }

        // get the map dimensions
        // if we are at capacity, we're done
        map_dimensions& dims = map_get_dimensions(map_index);
        if (dims.count_chunks == cfg.map_chunk_capacity) {
            return(INVALID_INDEX);
        }

        // the chunk has to fit on the map
        const bool is_on_map = (
            (origin_row + count_rows) <= dims.count_rows &&
            (origin_col + count_cols) <= dims.count_cols
        );
        if (!is_on_map) {
            return(INVALID_INDEX);
        }

        // chunks can't overlap, every tile belongs to one chunk
        for (
            u32 i_other = 0;
                i_other < dims.count_chunks;
              ++i_other) {

            const map_chunk& other = map_get_chunk(map_index, i_other);
            const bool is_overlapping = (
                origin_row < (other.origin_row + other.count_rows) && other.origin_row < (origin_row + count_rows) &&
                origin_col < (other.origin_col + other.count_cols) && other.origin_col < (origin_col + count_cols)
            );
            if (is_overlapping) {
                return(INVALID_INDEX);
            }
        }

        // the chunk's floor tiles have to fit in the map's floor array
        map_floor_array& floor_array      = map_get_floor_array(map_index);
        const u32        chunk_tile_count = count_rows * count_cols;
        if ((floor_array.count + chunk_tile_count) > cfg.map_tile_capacity) {
            return(INVALID_INDEX);
        }

        // the chunk's walls have to fit in the map's wall arrays
        // walls sit on the tile edges, so there's one more line of them than tiles
        map_wall_array& row_wall_array    = map_get_wall_array(map_index, map_wall_orientation_e_row);
        map_wall_array& column_wall_array = map_get_wall_array(map_index, map_wall_orientation_e_column);
        const u32       row_wall_count    = (count_rows + 1) * count_cols;
        const u32       column_wall_count = count_rows * (count_cols + 1);
        const bool      do_walls_fit      = (
            (row_wall_array.count    + row_wall_count)    <= cfg.map_wall_capacity &&
            (column_wall_array.count + column_wall_count) <= cfg.map_wall_capacity
        );
        if (!do_walls_fit) {
            return(INVALID_INDEX);
        }

        // get the chunk
        const u32  chunk_index = dims.count_chunks;
        map_chunk& chunk       = map_get_chunk(map_index, chunk_index);
    
        // set the chunk properties
        chunk.origin_row = origin_row;
        chunk.origin_col = origin_col;
        chunk.count_rows = count_rows;
        chunk.count_cols = count_cols;
        chunk.base_color = base_color;
        chunk.h_atlas      = INVALID_HANDLE;
        chunk.floor        = &floor_array.tiles[floor_array.count];
        chunk.row_walls    = &row_wall_array.tiles[row_wall_array.count];
        chunk.column_walls = &column_wall_array.tiles[column_wall_array.count];

        // initialize the floor tiles
        // no unnavigable tiles yet
        for (
            u32 chunk_tile_index = 0;
                chunk_tile_index < chunk_tile_count;
              ++chunk_tile_index) {

            map_floor_tile& tile = chunk.floor[chunk_tile_index];
            tile.i_sprite = INVALID_INDEX;
            tile.color    = (u8)base_color.val;
            tile.flags    = (u8)map_tile_flag_e_navigable;
        }
        floor_array.count += chunk_tile_count;

        // initialize the walls
        // nothing is placed until it's set
        for (
            u32 wall_index = 0;
                wall_index < row_wall_count;
              ++wall_index) {

            map_wall_tile& wall = chunk.row_walls[wall_index];
            wall.base_color = base_color;
            wall.i_sprite   = INVALID_INDEX;
            wall.flags      = 0;
        }
        for (
            u32 wall_index = 0;
                wall_index < column_wall_count;
              ++wall_index) {

            map_wall_tile& wall = chunk.column_walls[wall_index];
            wall.base_color = base_color;
            wall.i_sprite   = INVALID_INDEX;
            wall.flags      = 0;
        }
        row_wall_array.count    += row_wall_count;
        column_wall_array.count += column_wall_count;

        // update the chunk count
        ++dims.count_chunks;

        return(chunk_index);
    }

    IFB_INTERNAL bool
    map_chunk_get_dimensions(
        const hnd_map map_hnd,
        const u32     chunk_index,
        u32&          count_rows,
        u32&          count_cols,
        u32&          origin_row,
        u32&          origin_col) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks) {
            return(false);
        }

        const map_chunk& chunk = map_get_chunk(map_index, chunk_index);
        count_rows = chunk.count_rows;
        count_cols = chunk.count_cols;
        origin_row = chunk.origin_row;
        origin_col = chunk.origin_col;
        return(true);
    }

    // removes a chunk and its tiles and walls, the chunks after it move down one index
    // chunks are packed in order in the map's floor and wall arrays, so the ones after it slide down to fill the gap
    IFB_INTERNAL bool
    map_chunk_destroy(
        const hnd_map map_hnd,
        const u32     chunk_index) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks) {
            return(false);
        }

        map_chunk& chunk = map_get_chunk(map_index, chunk_index);

        // an edge between two chunks is in both of them, but a wall on it is only placed in the first one
        // so before the chunk goes, its walls on shared edges move to the other chunk that has the edge
        for (u32 i_orientation = 0; i_orientation < 2; ++i_orientation) {

            const map_wall_orientation_e orientation = (i_orientation == 0)
                ? map_wall_orientation_e_row
                : map_wall_orientation_e_column;
            const bool is_row    = (orientation == map_wall_orientation_e_row);
            const u32  wall_rows = is_row ? (chunk.count_rows + 1) : chunk.count_rows;
            const u32  wall_cols = is_row ? chunk.count_cols       : (chunk.count_cols + 1);

            for (u32 i_row = 0; i_row < wall_rows; ++i_row) {
                for (u32 i_col = 0; i_col < wall_cols; ++i_col) {

                    const s32 row = (s32)(chunk.origin_row + i_row);
                    const s32 col = (s32)(chunk.origin_col + i_col);

                    const map_wall_tile* wall = map_chunk_get_wall(chunk, row, col, orientation);
                    if (wall == NULL || (wall->flags & (u8)map_wall_flag_e_placed) == 0) continue;

                    for (u32 i_other = 0; i_other < dims.count_chunks; ++i_other) {
                        if (i_other == chunk_index) continue;

                        map_chunk&     other      = map_get_chunk(map_index, i_other);
                        map_wall_tile* other_wall = map_chunk_get_wall(other, row, col, orientation);
                        if (other_wall == NULL) continue;

                        // a sprite only carries over if the other chunk draws from the same atlas
                        *other_wall = *wall;
                        if (other_wall->i_sprite != INVALID_INDEX && other.h_atlas != chunk.h_atlas) {
                            if (other.h_atlas == INVALID_HANDLE) other.h_atlas = chunk.h_atlas;
                            else                                 other_wall->i_sprite = INVALID_INDEX;
                        }
                        break;
                    }
                }
            }
        }

        const u32 tile_count        = chunk.count_rows * chunk.count_cols;
        const u32 row_wall_count    = (chunk.count_rows + 1) * chunk.count_cols;
        const u32 column_wall_count = chunk.count_rows * (chunk.count_cols + 1);

        map_floor_array& floor_array       = map_get_floor_array (map_index);
        map_wall_array&  row_wall_array    = map_get_wall_array  (map_index, map_wall_orientation_e_row);
        map_wall_array&  column_wall_array = map_get_wall_array  (map_index, map_wall_orientation_e_column);

        // slide everything after the chunk's tiles and walls down over them
        const u32 floor_start      = (u32)(chunk.floor        - floor_array.tiles);
        const u32 row_wall_start   = (u32)(chunk.row_walls    - row_wall_array.tiles);
        const u32 column_wall_start= (u32)(chunk.column_walls - column_wall_array.tiles);

        memmove(
            &floor_array.tiles[floor_start],
            &floor_array.tiles[floor_start + tile_count],
            (floor_array.count - (floor_start + tile_count)) * sizeof(map_floor_tile));
        memmove(
            &row_wall_array.tiles[row_wall_start],
            &row_wall_array.tiles[row_wall_start + row_wall_count],
            (row_wall_array.count - (row_wall_start + row_wall_count)) * sizeof(map_wall_tile));
        memmove(
            &column_wall_array.tiles[column_wall_start],
            &column_wall_array.tiles[column_wall_start + column_wall_count],
            (column_wall_array.count - (column_wall_start + column_wall_count)) * sizeof(map_wall_tile));

        floor_array.count       -= tile_count;
        row_wall_array.count    -= row_wall_count;
        column_wall_array.count -= column_wall_count;

        // the chunks after it point at their moved tiles and walls, and move down one index
        for (u32 i_next = chunk_index + 1; i_next < dims.count_chunks; ++i_next) {

            map_chunk& next = map_get_chunk(map_index, i_next);
            next.floor        -= tile_count;
            next.row_walls    -= row_wall_count;
            next.column_walls -= column_wall_count;

            map_get_chunk(map_index, i_next - 1) = next;
        }

        --dims.count_chunks;
        return(true);
    }

    // the chunk's color, which its tiles without sprites show, and which tints the ones with sprites
    IFB_INTERNAL bool
    map_chunk_set_color(
        const hnd_map       map_hnd,
        const u32           chunk_index,
        const map_color_u32 color) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks || color.val > (u32)map_color_e_none) {
            return(false);
        }

        map_chunk& chunk = map_get_chunk(map_index, chunk_index);
        chunk.base_color = color;

        const u32 tile_count = chunk.count_rows * chunk.count_cols;
        for (u32 i_tile = 0; i_tile < tile_count; ++i_tile) {
            chunk.floor[i_tile].color = (u8)color.val;
        }
        return(true);
    }

    IFB_INTERNAL bool
    map_chunk_get_info(
        const hnd_map   map_hnd,
        const u32       chunk_index,
        map_chunk_info& info) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks) {
            return(false);
        }

        const map_chunk& chunk = map_get_chunk(map_index, chunk_index);
        info.origin_row = chunk.origin_row;
        info.origin_col = chunk.origin_col;
        info.count_rows = chunk.count_rows;
        info.count_cols = chunk.count_cols;
        info.color      = chunk.base_color.val;
        info.h_atlas    = chunk.h_atlas;
        return(true);
    }

    IFB_INTERNAL u32
    map_get_chunk_count(
        const hnd_map map_hnd) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(0);
        }

        return(map_get_dimensions(map_index).count_chunks);
    }
};
