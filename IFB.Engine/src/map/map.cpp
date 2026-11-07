#pragma once

#include "component-tables.cpp"
#include "eng-json.hpp"
#include "eng-memory.hpp"
#include "ifb-config.hpp"
#include "eng-map.hpp"
#include "ifb.hpp"
#include "json.cpp"
#include "memory-arena.cpp"
#include "sld.hpp"
#include "eng-sprite.hpp"
#include <cassert>
#include <cmath>
#include <sld-strings.hpp>
#include "ifb-engine.hpp"
#include "eng-entity.hpp"
#include "eng-asset.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace ifb {

    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL hnd_map
    map_create(
        const cchar* map_name,
        const u32    count_rows,
        const u32    count_cols) {

        const auto& cfg = config_instance();
        assert(_map_mngr);
        map_table* tbl_map = _map_mngr->tbl_map;
        assert(tbl_map);

        assert(map_name);
        assert(count_rows != 0);
        assert(count_cols != 0);

        // the renderer indexes tiles and walls in 16 bits, on a grid one bigger than the tiles
        if (!map_is_size_valid(count_rows, count_cols)) {
            return(INVALID_HANDLE);
        }

        // find an open index
        u32 index = INVALID_INDEX;
        for (
            u32 i = 0;
                i < cfg.map_capacity;
              ++i) {

            if (tbl_map->hnd[i] == INVALID_HANDLE) {
                index = i;
                break;
            }
        }

        // if we didn't find a free index, we're done
        if (index == INVALID_INDEX) {
            return(INVALID_HANDLE);
        }

        // get the map properties
        hnd_map&      hnd    = tbl_map->hnd         [index];
        map_dimensions&  dims   = tbl_map->dims        [index]; 
        cstr_c16&        name   = tbl_map->name        [index];
        map_chunk_array& chunks = tbl_map->chunk_array [index];
        map_floor_array& floor  = tbl_map->floor_array [index];
        vec3&            origin = tbl_map->origin      [index];
        map_wall_array&  row_walls    = tbl_map->row_wall_array    [index];
        map_wall_array&  column_walls = tbl_map->column_wall_array [index];

        // the handle hashes all 16 bytes, so a reused slot can't keep the end of an old name
        zero_memory((void*)&name, sizeof(name));
        cstr_c16_init(&name, map_name);
        hnd = cstr_c16_hash(&name);
        dims.count_chunks = 0;
        dims.count_rows   = count_rows;
        dims.count_cols   = count_cols;
        floor.count       = 0;
        row_walls.count    = 0;
        column_walls.count = 0;
        origin.x          = 0.0f;
        origin.y          = 0.0f;
        origin.z          = 0.0f;

        return(hnd);
    }

    IFB_INTERNAL bool 
    map_destroy(
        const hnd_map map_hnd) {

        const auto& cfg = config_instance();
       
        assert(map_hnd != INVALID_HANDLE);
      
        assert(_map_mngr);
        map_table* tbl_map = _map_mngr->tbl_map;
        assert(tbl_map); 

        const u32  map_index = map_lookup_index(map_hnd);
        const bool result    = (map_index != INVALID_INDEX);

        if (result) {
            tbl_map->hnd[map_index] = INVALID_HANDLE;
        }

        return(result);
    }
    
    IFB_INTERNAL map_dimensions&
    map_get_dimensions(
        const u32 map_index) {

        assert(_map_mngr);
        map_table* map_tbl = _map_mngr->tbl_map;
        assert(map_tbl != NULL);

        const auto& cfg = config_instance();

        assert(map_index < cfg.map_capacity);
        assert(map_tbl->hnd[map_index] != INVALID_HANDLE);

        return(map_tbl->dims[map_index]);
    }
    
    IFB_INTERNAL u32
    map_render_buffer(
        byte*         buffer_data,
        const u32     buffer_size,
        const hnd_map map_hnd) {

        // validate
        assert(buffer_data        != NULL);
        assert(buffer_size        != 0);
        assert(map_hnd            != INVALID_HANDLE);
        assert(_map_mngr          != NULL);
        assert(_map_mngr->tbl_map != NULL);

        // get the map index
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(0);
        }

        // get the map dimensions
        const map_dimensions& dims = map_get_dimensions(map_index);

        // count the floor tiles and placed walls
        // an edge between two chunks is only ever placed in one of them,
        // so each wall is only counted once
        u32 tile_count = 0;
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk = map_get_chunk(map_index, chunk_index);
            tile_count += (chunk.count_rows * chunk.count_cols);

            const u32 row_wall_count    = (chunk.count_rows + 1) * chunk.count_cols;
            const u32 column_wall_count = chunk.count_rows * (chunk.count_cols + 1);
            for (u32 wall_index = 0; wall_index < row_wall_count;    ++wall_index) if ((chunk.row_walls   [wall_index].flags & (u8)map_wall_flag_e_placed) != 0) ++tile_count;
            for (u32 wall_index = 0; wall_index < column_wall_count; ++wall_index) if ((chunk.column_walls[wall_index].flags & (u8)map_wall_flag_e_placed) != 0) ++tile_count;
        }
        if (tile_count == 0) {
            return(0);
        }

        // walls sit on the edges, so they're indexed on a grid one bigger than the tiles
        // and both grids have to fit in the tile index
        assert(((dims.count_rows + 1) * (dims.count_cols + 1)) <= 0xFFFF);

        // calculate the size needed to render the buffer,
        // and make sure the buffer is large enough
        const u32 render_size = sizeof(map_tile) * tile_count;
        assert(buffer_size >= render_size);
        auto tile_buffer = (map_tile*)buffer_data;

        // fill out the render buffer
        u32 tile_index = 0;
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk            = map_get_chunk(map_index, chunk_index);
            const u32        chunk_tile_count = chunk.count_rows * chunk.count_cols;
            assert(chunk.floor);

            // neighboring tiles usually share a sprite,
            // so only look up the rect when it changes
            u32  i_sprite_prev       = INVALID_INDEX;
            bool did_get_sprite_info = false;
            f32  u_left              = 0.0f;
            f32  u_right             = 1.0f;
            f32  v_top               = 0.0f;
            f32  v_bottom            = 1.0f;

            for (
                u32 chunk_tile_index = 0;
                    chunk_tile_index < chunk_tile_count;
                  ++chunk_tile_index) {

                const map_floor_tile& floor_tile = chunk.floor[chunk_tile_index];

                // the sprite's rect in the atlas, normalized
                // the atlas isn't flipped on upload, so the top row is v = 0
                // with no sprite, the tile is a solid color
                if (floor_tile.i_sprite != i_sprite_prev) {

                    i_sprite_prev       = floor_tile.i_sprite;
                    did_get_sprite_info = map_get_sprite_rect(chunk.h_atlas, floor_tile.i_sprite, u_left, v_top, u_right, v_bottom);
                }

                // calculate the row and column
                const u32 row = (chunk_tile_index / chunk.count_cols) + chunk.origin_row;
                const u32 col = (chunk_tile_index % chunk.count_cols) + chunk.origin_col;

                // get the next tile
                map_tile& tile = tile_buffer[tile_index++];

                // initialize the tile
                // only draw it textured if the sprite actually resolved
                tile.index    = (row * dims.count_cols) + col;
                tile.color    = floor_tile.color;
                tile.flags    = floor_tile.flags;
                tile.flags   &= (u8)~map_tile_flag_e_textured;
                if (did_get_sprite_info) {
                    tile.flags |= (u8)map_tile_flag_e_textured;
                }
                tile.u_left   = u_left;
                tile.v_top    = v_top;
                tile.u_right  = u_right;
                tile.v_bottom = v_bottom;
            }
        }

        // the walls go after the floor, the shader stands them up on their edge
        const u32 wall_stride = dims.count_cols + 1;
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk = map_get_chunk(map_index, chunk_index);

            for (
                u32 orientation_index = 0;
                    orientation_index < 2;
                  ++orientation_index) {

                const bool           is_row     = (orientation_index == map_wall_orientation_e_row);
                const map_wall_tile* walls      = is_row ? chunk.row_walls : chunk.column_walls;
                const u32            wall_rows  = is_row ? (chunk.count_rows + 1) : chunk.count_rows;
                const u32            wall_cols  = is_row ? chunk.count_cols       : (chunk.count_cols + 1);
                const u32            wall_count = wall_rows * wall_cols;
                const u8             wall_flag  = is_row ? (u8)map_tile_flag_e_wall_row : (u8)map_tile_flag_e_wall_column;
                assert(walls);

                for (
                    u32 wall_index = 0;
                        wall_index < wall_count;
                      ++wall_index) {

                    const map_wall_tile& wall = walls[wall_index];
                    if ((wall.flags & (u8)map_wall_flag_e_placed) == 0) continue;

                    f32 u_left, v_top, u_right, v_bottom;
                    const bool did_get_sprite_info = map_get_sprite_rect(chunk.h_atlas, wall.i_sprite, u_left, v_top, u_right, v_bottom);

                    // calculate the row and column of the edge
                    const u32 row = (wall_index / wall_cols) + chunk.origin_row;
                    const u32 col = (wall_index % wall_cols) + chunk.origin_col;

                    // get the next tile
                    map_tile& tile = tile_buffer[tile_index++];

                    // initialize the tile
                    tile.index    = (row * wall_stride) + col;
                    tile.color    = (u8)wall.base_color.val;
                    tile.flags    = wall_flag;
                    if (did_get_sprite_info) {
                        tile.flags |= (u8)map_tile_flag_e_textured;
                    }
                    tile.u_left   = u_left;
                    tile.v_top    = v_top;
                    tile.u_right  = u_right;
                    tile.v_bottom = v_bottom;
                }
            }
        }

        assert(tile_index == tile_count);
        return(render_size);
    }

    IFB_INTERNAL bool
    map_get_pos_from_coords(
        const cmpnt_map_coords& coords,
              cmpnt_position&   pos) {

        const auto& cfg = config_instance();

        // get the map origin
        vec3 origin;
        if(!map_get_origin(coords.h_map, origin)) {
            return(false);
        }

        map_calculate_position(
            cfg.map_tile_unit_size,
            origin,
            coords,
            pos
        );

        return(true);
    }

    IFB_INTERNAL bool
    map_get_coords_from_pos(
        const cmpnt_position&   pos,
              cmpnt_map_coords& coords) {

        const auto& cfg = config_instance();

        // get the map origin
        vec3 origin;
        if (!map_get_origin(coords.h_map, origin)) {
            return(false);
        }

        map_calculate_coordinates(
            cfg.map_tile_unit_size,
            origin,
            pos,
            coords
        );

        return(true);
    }

    IFB_INTERNAL bool
    map_is_in_bounds(
        const cmpnt_map_coords& coords) {

        const u32 map_index = map_lookup_index(coords.h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // the coordinates have to be on the map
        const map_dimensions& dims = map_get_dimensions(map_index);
        if (!map_calculate_is_in_bounds(dims, coords)) {
            return(false);
        }

        // the chunks are the navigable portions of the map,
        // so the coordinates have to land on one of them
        const bool is_in_bounds = map_is_tile_navigable(
            map_index,
            (s32)floorf(coords.row_z),
            (s32)floorf(coords.col_x)
        );
        return(is_in_bounds);
    }

    IFB_INTERNAL bool
    map_constrain_movement(
        const hnd_map   map_hnd,
        const f32       inset,
        cmpnt_position& pos,
        cmpnt_velocity& vel) {

        const auto& cfg = config_instance();

        // get the map origin and dimensions
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }
        const vec3&           origin = _map_mngr->tbl_map->origin[map_index];
        const map_dimensions& dims   = map_get_dimensions(map_index);
        if (dims.count_chunks == 0) {
            return(false);
        }

        // work in tile units relative to the map origin,
        // the entity is a box that extends the inset out from its position
        const f32 tile_size = cfg.map_tile_unit_size;
        const f32 radius    = inset / tile_size;
        const f32 col_x     = (pos.x - origin.x) / tile_size;
        const f32 row_z     = (pos.z - origin.z) / tile_size;
        f32       push_x    = 0.0f;
        f32       push_z    = 0.0f;

        // if the position isn't on a chunk,
        // start by putting it on the edge of the nearest one
        if (!map_is_tile_navigable(map_index, (s32)floorf(row_z), (s32)floorf(col_x))) {

            f32 nearest_dist_sq = 0.0f;
            for (
                u32 chunk_index = 0;
                    chunk_index < dims.count_chunks;
                  ++chunk_index) {

                const map_chunk& chunk = map_get_chunk(map_index, chunk_index);

                const f32 chunk_x_min = (f32)chunk.origin_col;
                const f32 chunk_z_min = (f32)chunk.origin_row;
                const f32 chunk_x_max = (f32)(chunk.origin_col + chunk.count_cols);
                const f32 chunk_z_max = (f32)(chunk.origin_row + chunk.count_rows);

                const f32 nearest_x = (col_x < chunk_x_min) ? chunk_x_min : (col_x > chunk_x_max) ? chunk_x_max : col_x;
                const f32 nearest_z = (row_z < chunk_z_min) ? chunk_z_min : (row_z > chunk_z_max) ? chunk_z_max : row_z;
                const f32 dist_x    = nearest_x - col_x;
                const f32 dist_z    = nearest_z - row_z;
                const f32 dist_sq   = (dist_x * dist_x) + (dist_z * dist_z);

                if (chunk_index == 0 || dist_sq < nearest_dist_sq) {
                    nearest_dist_sq = dist_sq;
                    push_x          = dist_x;
                    push_z          = dist_z;
                }
            }
        }

        // push the box out of any tiles that aren't on a chunk, and out of any walls
        const f32 wall_t = MAP_WALL_HALF_THICKNESS;
        for (
            u32 iteration = 0;
                iteration < MAP_CONSTRAIN_ITERATION_COUNT;
              ++iteration) {

            const f32 box_x_min = col_x + push_x - radius;
            const f32 box_x_max = col_x + push_x + radius;
            const f32 box_z_min = row_z + push_z - radius;
            const f32 box_z_max = row_z + push_z + radius;

            const s32 col_min = (s32)floorf(box_x_min);
            const s32 row_min = (s32)floorf(box_z_min);
            const s32 col_max = (s32)ceilf(box_x_max) - 1;
            const s32 row_max = (s32)ceilf(box_z_max) - 1;

            // find the tile or wall the box overlaps the most,
            // resolving that one first keeps the box from
            // catching on the seams between tiles along an edge
            f32 overlap_area_max = 0.0f;
            f32 tile_push_x      = 0.0f;
            f32 tile_push_z      = 0.0f;
            for (s32 row = row_min; row <= row_max; ++row) {
                for (s32 col = col_min; col <= col_max; ++col) {

                    if (map_is_tile_navigable(map_index, row, col)) continue;

                    map_calculate_box_push(
                        box_x_min, box_z_min, box_x_max, box_z_max,
                        (f32)col, (f32)row, (f32)(col + 1), (f32)(row + 1),
                        overlap_area_max, tile_push_x, tile_push_z
                    );
                }
            }

            // row walls lie on the line z = row and run along x,
            // the box can touch the line on either side of its tiles
            // and the end of a wall from the neighboring column
            for (s32 row = row_min; row <= row_max + 1; ++row) {
                for (s32 col = col_min - 1; col <= col_max + 1; ++col) {

                    if (!map_has_wall(map_index, row, col, map_wall_orientation_e_row)) continue;

                    map_calculate_box_push(
                        box_x_min, box_z_min, box_x_max, box_z_max,
                        (f32)col - wall_t, (f32)row - wall_t, (f32)(col + 1) + wall_t, (f32)row + wall_t,
                        overlap_area_max, tile_push_x, tile_push_z
                    );
                }
            }

            // column walls lie on the line x = col and run along z
            for (s32 row = row_min - 1; row <= row_max + 1; ++row) {
                for (s32 col = col_min; col <= col_max + 1; ++col) {

                    if (!map_has_wall(map_index, row, col, map_wall_orientation_e_column)) continue;

                    map_calculate_box_push(
                        box_x_min, box_z_min, box_x_max, box_z_max,
                        (f32)col - wall_t, (f32)row - wall_t, (f32)col + wall_t, (f32)(row + 1) + wall_t,
                        overlap_area_max, tile_push_x, tile_push_z
                    );
                }
            }

            // if the box isn't overlapping anything, we're done
            if (overlap_area_max == 0.0f) break;

            push_x += tile_push_x;
            push_z += tile_push_z;
        }

        // put the position back on the chunks
        // and stop any movement off of the edge
        const bool did_constrain = (push_x != 0.0f || push_z != 0.0f);
        if (push_x != 0.0f) { pos.x += (push_x * tile_size); if ((push_x > 0.0f) == (vel.x < 0.0f)) vel.x = 0.0f; }
        if (push_z != 0.0f) { pos.z += (push_z * tile_size); if ((push_z > 0.0f) == (vel.z < 0.0f)) vel.z = 0.0f; }

        return(did_constrain);
    }

    IFB_INTERNAL bool
    map_get_navigable_bounds(
        const hnd_map map_hnd,
        u32&          row_min,
        u32&          col_min,
        u32&          row_max,
        u32&          col_max) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (dims.count_chunks == 0) {
            return(false);
        }

        // the bounds are the smallest area that fits every chunk
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk = map_get_chunk(map_index, chunk_index);

            const u32 chunk_row_max = chunk.origin_row + chunk.count_rows;
            const u32 chunk_col_max = chunk.origin_col + chunk.count_cols;

            if (chunk_index == 0 || chunk.origin_row < row_min) row_min = chunk.origin_row;
            if (chunk_index == 0 || chunk.origin_col < col_min) col_min = chunk.origin_col;
            if (chunk_index == 0 || chunk_row_max    > row_max) row_max = chunk_row_max;
            if (chunk_index == 0 || chunk_col_max    > col_max) col_max = chunk_col_max;
        }

        return(true);
    }

    IFB_INTERNAL bool
    map_get_origin(
        const hnd_map map_hnd,
        vec3&         origin) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        origin = _map_mngr->tbl_map->origin[map_index];
        return(true);
    }

    IFB_INTERNAL bool
    map_set_origin(
        const hnd_map map_hnd,
        const vec3&   origin) {

        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        _map_mngr->tbl_map->origin[map_index] = origin;
        return(true);
    }

    IFB_INTERNAL void
    map_entity_lookup_coords(
        const entity&     e,
        cmpnt_map_coords& coords) {

        // the component tells us which map the entity is on
        cmpnt_lookup_map_coords(e.index_sparse, coords);

        // the coordinates always come from the position
        if (e.archetype.has_all(cmpnt_type_e_position | cmpnt_type_e_map_coords)) {
            cmpnt_position pos;
            cmpnt_lookup_position         (e.index_sparse, pos);
            (void)map_get_coords_from_pos (pos, coords);
        }
    }

    IFB_INTERNAL void
    map_entity_update_coords(
        const entity&           e,
        const cmpnt_map_coords& coords) {

        cmpnt_update_map_coords(e.index_sparse, coords);

        // move the entity to the coordinates
        if (e.archetype.has_all(cmpnt_type_e_position | cmpnt_type_e_map_coords)) {
            cmpnt_position pos;
            if (map_get_pos_from_coords(coords, pos)) {
                cmpnt_update_position(e.index_sparse, pos);
            }
        }
    }

    IFB_INTERNAL entity_list*
    map_get_entities(
        const hnd_map   h_map,
        const hnd_arena h_arena) {
   
        assert(h_map   != INVALID_HANDLE);
        assert(h_arena != INVALID_HANDLE);

        // create the entity list
        entity_list* e_list = eng_entity_list_arena_create(h_arena);
        if (e_list == NULL) {
            return(NULL);
        }

        const u32 entity_count = entity_mngr_get_count();     
        entity           e;
        cmpnt_map_coords mc;
        for (
            u32 entity_index = 0;
                entity_index < entity_count;
              ++entity_index) {
    
            const bool did_find = entity_lookup_by_index_dense(e, entity_index); 
            assert(did_find);

            if (e.archetype.has_any(cmpnt_type_e_map_coords)) {
                cmpnt_lookup_map_coords(e.index_sparse, mc);
                if (mc.h_map == h_map) {
                    (void)eng_entity_list_add(e_list, e.id);     
                }    
            } 
        }

        return(e_list);
    }

    // entities whose map coordinates are on a tile covered by the chunk
    // returns NULL if the map or chunk is invalid, or the list can't be allocated
    IFB_INTERNAL entity_list*
    map_get_entities_in_chunk(
        const hnd_map   h_map,
        const u32       chunk_index,
        const hnd_arena h_arena) {

        assert(h_map   != INVALID_HANDLE);
        assert(h_arena != INVALID_HANDLE);

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(NULL);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks) {
            return(NULL);
        }

        // create the entity list
        entity_list* e_list = eng_entity_list_arena_create(h_arena);
        if (e_list == NULL) {
            return(NULL);
        }

        // the chunk's tile bounds, max is exclusive
        const map_chunk& chunk   = map_get_chunk(map_index, chunk_index);
        const f32        row_min = (f32)chunk.origin_row;
        const f32        col_min = (f32)chunk.origin_col;
        const f32        row_max = (f32)(chunk.origin_row + chunk.count_rows);
        const f32        col_max = (f32)(chunk.origin_col + chunk.count_cols);

        const u32 entity_count = entity_mngr_get_count();
        entity           e;
        cmpnt_map_coords mc;
        for (
            u32 entity_index = 0;
                entity_index < entity_count;
              ++entity_index) {

            const bool did_find = entity_lookup_by_index_dense(e, entity_index);
            assert(did_find);

            if (!e.archetype.has_any(cmpnt_type_e_map_coords)) {
                continue;
            }

            cmpnt_lookup_map_coords(e.index_sparse, mc);
            if (mc.h_map != h_map) {
                continue;
            }

            const bool is_row_in_chunk = (mc.row_z >= row_min && mc.row_z < row_max);
            const bool is_col_in_chunk = (mc.col_x >= col_min && mc.col_x < col_max);
            if (is_row_in_chunk && is_col_in_chunk) {
                (void)eng_entity_list_add(e_list, e.id);
            }
        }

        return(e_list);
    }

    IFB_INTERNAL bool
    map_tile_set_sprite(
        const hnd_map h_map,
        const u32     row,
        const u32     col,
        const cchar*  atlas_name_cstr,
        const cchar*  sprite_name_cstr) {

        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // the tile has to be on a chunk
        map_chunk*      chunk      = NULL;
        map_floor_tile* floor_tile = map_find_floor_tile(map_index, (s32)row, (s32)col, &chunk);
        if (floor_tile == NULL) {
            return(false);
        }

        // the atlas has to be loaded and have the sprite
        const hnd_sprite_atlas h_atlas  = sprite_mngr_lookup_atlas        (atlas_name_cstr);
        const u32              i_sprite = sprite_mngr_lookup_sprite_index (h_atlas, sprite_name_cstr);
        if (h_atlas == INVALID_HANDLE || i_sprite == INVALID_INDEX) {
            return(false);
        }

        // a chunk only has one atlas, so every sprite in it has to come from the same one
        if (chunk->h_atlas != INVALID_HANDLE && chunk->h_atlas != h_atlas) {
            return(false);
        }

        chunk->h_atlas       = h_atlas;
        floor_tile->i_sprite = i_sprite;
        floor_tile->flags   |= (u8)map_tile_flag_e_textured;
        return(true);
    }

    // the tile goes back to its chunk's color, like -1 in a map file
    IFB_INTERNAL bool
    map_tile_clear_sprite(
        const hnd_map h_map,
        const u32     row,
        const u32     col) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        map_floor_tile* floor_tile = map_find_floor_tile(map_index, (s32)row, (s32)col, NULL);
        if (floor_tile == NULL) {
            return(false);
        }

        // the chunk keeps its atlas, so its other tiles don't change
        floor_tile->i_sprite = INVALID_INDEX;
        floor_tile->flags   &= (u8)~((u8)map_tile_flag_e_textured);
        return(true);
    }

    // false if no chunk covers the tile
    IFB_INTERNAL bool
    map_tile_get_info(
        const hnd_map  h_map,
        const u32      row,
        const u32      col,
        map_tile_info& info) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        map_chunk*            chunk      = NULL;
        const map_floor_tile* floor_tile = map_find_floor_tile(map_index, (s32)row, (s32)col, &chunk);
        if (floor_tile == NULL || chunk == NULL) {
            return(false);
        }

        // the chunks are in an array, so the chunk's index is how far it is from the first
        const map_chunk& chunk_first = map_get_chunk(map_index, 0);
        info.chunk_index = (u32)(chunk - &chunk_first);
        info.h_atlas     = chunk->h_atlas;
        info.i_sprite    = floor_tile->i_sprite;
        info.color       = floor_tile->color;
        info.is_textured = (floor_tile->flags & (u8)map_tile_flag_e_textured) != 0;
        return(true);
    }

    IFB_INTERNAL bool
    map_chunk_set_sprite(
        const hnd_map h_map,
        const u32     chunk_index,
        const cchar*  atlas_name_cstr,
        const cchar*  sprite_name_cstr) {

        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const map_dimensions& dims = map_get_dimensions(map_index);
        if (chunk_index >= dims.count_chunks) {
            return(false);
        }

        // the atlas has to be loaded and have the sprite
        const hnd_sprite_atlas h_atlas  = sprite_mngr_lookup_atlas        (atlas_name_cstr);
        const u32              i_sprite = sprite_mngr_lookup_sprite_index (h_atlas, sprite_name_cstr);
        if (h_atlas == INVALID_HANDLE || i_sprite == INVALID_INDEX) {
            return(false);
        }

        // this replaces every tile's sprite, so the chunk can switch atlases
        map_chunk& chunk            = map_get_chunk(map_index, chunk_index);
        const u32  chunk_tile_count = chunk.count_rows * chunk.count_cols;
        chunk.h_atlas = h_atlas;
        for (
            u32 chunk_tile_index = 0;
                chunk_tile_index < chunk_tile_count;
              ++chunk_tile_index) {

            map_floor_tile& floor_tile = chunk.floor[chunk_tile_index];
            floor_tile.i_sprite = i_sprite;
            floor_tile.flags   |= (u8)map_tile_flag_e_textured;
        }
        return(true);
    }

    IFB_INTERNAL bool
    map_wall_set(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        const map_color_u32          color) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // the wall has to be on the edge of a chunk tile
        map_wall_tile* wall = map_find_wall(map_index, (s32)row, (s32)col, orientation, NULL);
        if (wall == NULL) {
            return(false);
        }

        // a solid wall drops any sprite it had
        wall->base_color = color;
        wall->i_sprite   = INVALID_INDEX;
        wall->flags      = (u8)map_wall_flag_e_placed;
        return(true);
    }

    IFB_INTERNAL bool
    map_wall_set_sprite(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        const cchar*                 atlas_name_cstr,
        const cchar*                 sprite_name_cstr) {

        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // the wall has to be on the edge of a chunk tile
        map_chunk*     chunk = NULL;
        map_wall_tile* wall  = map_find_wall(map_index, (s32)row, (s32)col, orientation, &chunk);
        if (wall == NULL) {
            return(false);
        }

        // the atlas has to be loaded and have the sprite
        const hnd_sprite_atlas h_atlas  = sprite_mngr_lookup_atlas        (atlas_name_cstr);
        const u32              i_sprite = sprite_mngr_lookup_sprite_index (h_atlas, sprite_name_cstr);
        if (h_atlas == INVALID_HANDLE || i_sprite == INVALID_INDEX) {
            return(false);
        }

        // walls share the chunk's atlas with the floor
        if (chunk->h_atlas != INVALID_HANDLE && chunk->h_atlas != h_atlas) {
            return(false);
        }

        chunk->h_atlas = h_atlas;
        wall->i_sprite = i_sprite;
        wall->flags    = (u8)(map_wall_flag_e_placed | map_wall_flag_e_textured);
        return(true);
    }

    IFB_INTERNAL bool
    map_wall_clear(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // an edge between two chunks is in both of them, so clear it everywhere
        bool did_clear = false;
        const map_dimensions& dims = map_get_dimensions(map_index);
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            map_chunk&     chunk = map_get_chunk(map_index, chunk_index);
            map_wall_tile* wall  = map_chunk_get_wall(chunk, (s32)row, (s32)col, orientation);
            if (wall != NULL) {
                wall->i_sprite = INVALID_INDEX;
                wall->flags    = 0;
                did_clear      = true;
            }
        }
        return(did_clear);
    }

    IFB_INTERNAL bool
    map_wall_is_placed(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        const bool is_placed = map_has_wall(map_index, (s32)row, (s32)col, orientation);
        return(is_placed);
    }

    // the wall on an edge, read from the chunk that owns the edge, which is the one map_wall_set writes to
    // false if no chunk has the edge
    IFB_INTERNAL bool
    map_wall_get_info(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        map_wall_info&               info) {

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        map_chunk*           chunk = NULL;
        const map_wall_tile* wall  = map_find_wall(map_index, (s32)row, (s32)col, orientation, &chunk);
        if (wall == NULL || chunk == NULL) {
            return(false);
        }

        const map_chunk& chunk_first = map_get_chunk(map_index, 0);
        info.chunk_index = (u32)(chunk - &chunk_first);
        info.h_atlas     = chunk->h_atlas;
        info.i_sprite    = wall->i_sprite;
        info.color       = wall->base_color.val;
        info.is_placed   = (wall->flags & (u8)map_wall_flag_e_placed) != 0;
        return(true);
    }
   
    // false if the string isn't one of the color names
    inline bool
    map_parse_color_from_cstr(
        const cchar*   cstr,
        map_color_u32& color) {

        if (cstr == NULL) return(false);

        static const u32 hash_color_none         = 0xA5447BC9; // "none"        
        static const u32 hash_color_red_light    = 0xBB5A31FE; // "red_light"   
        static const u32 hash_color_red_dark     = 0xC5B3AA05; // "red_dark"    
        static const u32 hash_color_orange_light = 0x4F626D22; // "orange_light"
        static const u32 hash_color_orange_dark  = 0x90F57670; // "orange_dark" 
        static const u32 hash_color_yellow_light = 0x3C1E4D9E; // "yellow_light"
        static const u32 hash_color_yellow_dark  = 0xCCBEF4B1; // "yellow_dark" 
        static const u32 hash_color_green_light  = 0x049E82C8; // "green_light" 
        static const u32 hash_color_green_dark   = 0xDB369439; // "green_dark"  
        static const u32 hash_color_aqua_light   = 0xA10F1ACC; // "aqua_light"  
        static const u32 hash_color_aqua_dark    = 0x8C72645F; // "aqua_dark"   
        static const u32 hash_color_blue_light   = 0x85FA5873; // "blue_light"  
        static const u32 hash_color_blue_dark    = 0x669C41FF; // "blue_dark"   
        static const u32 hash_color_purple_light = 0x27749FD3; // "purple_light"
        static const u32 hash_color_purple_dark  = 0xBCAA404F; // "purple_dark" 
        static const u32 hash_color_gray_light   = 0x8534C8FA; // "gray_light"  
        static const u32 hash_color_gray_dark    = 0xF4149944; // "gray_dark"   

        // hash the color string, the longest name is 12 characters
        const u32 cstr_len = cstr_nvar_length(cstr, 16);
        if (cstr_len == 0) return(false);
        const u32 cstr_hash = cstr_nvar_hash(cstr, cstr_len);

        // get the color
        switch(cstr_hash) {
            case(hash_color_red_light):     color = map_color_e_red_light;    return(true);
            case(hash_color_red_dark):      color = map_color_e_red_dark;     return(true);
            case(hash_color_orange_light):  color = map_color_e_orange_light; return(true);
            case(hash_color_orange_dark):   color = map_color_e_orange_dark;  return(true);
            case(hash_color_yellow_light):  color = map_color_e_yellow_light; return(true);
            case(hash_color_yellow_dark):   color = map_color_e_yellow_dark;  return(true);
            case(hash_color_green_light):   color = map_color_e_green_light;  return(true);
            case(hash_color_green_dark):    color = map_color_e_green_dark;   return(true);
            case(hash_color_aqua_light):    color = map_color_e_aqua_light;   return(true);
            case(hash_color_aqua_dark):     color = map_color_e_aqua_dark;    return(true);
            case(hash_color_blue_light):    color = map_color_e_blue_light;   return(true);
            case(hash_color_blue_dark):     color = map_color_e_blue_dark;    return(true);
            case(hash_color_purple_light):  color = map_color_e_purple_light; return(true);
            case(hash_color_purple_dark):   color = map_color_e_purple_dark;  return(true);
            case(hash_color_gray_light):    color = map_color_e_gray_light;   return(true);
            case(hash_color_gray_dark):     color = map_color_e_gray_dark;    return(true);
            case(hash_color_none):          color = map_color_e_none;         return(true);
            default:                        return(false);
        }
    }

    // one entry in the chunks array, see docs/map-format.md
    inline bool
    map_parse_chunk_from_json(
        const hnd_map   h_map,
        const json_obj* chunk_obj) {

        if (chunk_obj == NULL) return(false);

        // the chunk's area and color are required
        u32           chunk_map_row   = 0;
        u32           chunk_map_col   = 0;
        u32           chunk_row_count = 0;
        u32           chunk_col_count = 0;
        const cchar*  chunk_color_cstr = NULL;
        map_color_u32 chunk_color;

        const bool has_chunk = (
            json_obj_try_get_u32      (chunk_obj, "row",   chunk_map_row)    &&
            json_obj_try_get_u32      (chunk_obj, "col",   chunk_map_col)    &&
            json_obj_try_get_u32      (chunk_obj, "rows",  chunk_row_count)  &&
            json_obj_try_get_u32      (chunk_obj, "cols",  chunk_col_count)  &&
            json_obj_try_get_cstr     (chunk_obj, "color", chunk_color_cstr) &&
            map_parse_color_from_cstr (chunk_color_cstr,   chunk_color)      &&
            chunk_row_count > 0 &&
            chunk_col_count > 0
        );
        if (!has_chunk) return(false);

        const u32 chunk_index = map_chunk_create(
            h_map,
            chunk_row_count,
            chunk_col_count,
            chunk_map_row,
            chunk_map_col,
            chunk_color
        );
        if (chunk_index == INVALID_INDEX) return(false);

        // atlas, sprites, and tiles come together
        // a chunk with none of them is a solid color
        const bool has_atlas   = json_obj_has_member(chunk_obj, "atlas");
        const bool has_sprites = json_obj_has_member(chunk_obj, "sprites");
        const bool has_tiles   = json_obj_has_member(chunk_obj, "tiles");
        if (!has_atlas && !has_sprites && !has_tiles) return(true);
        if (!has_atlas || !has_sprites || !has_tiles) return(false);

        const cchar*    chunk_atlas   = NULL;
        const json_arr* chunk_sprites = json_obj_get_arr(chunk_obj, "sprites");
        const json_arr* chunk_tiles   = json_obj_get_arr(chunk_obj, "tiles");
        const bool      is_valid      = (
            json_obj_try_get_cstr(chunk_obj, "atlas", chunk_atlas) &&
            chunk_sprites != NULL &&
            chunk_tiles   != NULL
        );
        if (!is_valid) return(false);

        // one tile entry per chunk tile, and at least one sprite for them to use
        const u32 chunk_sprite_count = json_arr_get_count(chunk_sprites);
        const u32 chunk_tile_count   = chunk_row_count * chunk_col_count;
        if (chunk_sprite_count == 0)                             return(false);
        if (json_arr_get_count(chunk_tiles) != chunk_tile_count) return(false);

        for (
            u32 i_row = 0;
                i_row < chunk_row_count;
              ++i_row) {

            for (
                u32 i_col = 0;
                    i_col < chunk_col_count;
                  ++i_col) {

                // tiles are row-major, each entry is an index into sprites
                const u32 i_tile   = (i_row * chunk_col_count) + i_col;
                s32       i_sprite = -1;
                if (!json_arr_try_get_s32(chunk_tiles, i_tile, i_sprite)) return(false);

                // -1 keeps the chunk's color
                if (i_sprite == -1) continue;
                if (i_sprite < 0 || (u32)i_sprite >= chunk_sprite_count) return(false);

                const cchar* sprite_cstr = NULL;
                if (!json_arr_try_get_cstr(chunk_sprites, (u32)i_sprite, sprite_cstr)) return(false);

                // tiles are set by map coordinates, not chunk coordinates
                const bool did_set = map_tile_set_sprite(
                    h_map,
                    chunk_map_row + i_row,
                    chunk_map_col + i_col,
                    chunk_atlas,
                    sprite_cstr
                );
                if (!did_set) return(false);
            }
        }

        return(true);
    }

    // one entry in the walls array, a straight run of walls, see docs/map-format.md
    inline bool
    map_parse_wall_from_json(
        const hnd_map   h_map,
        const json_obj* wall_obj) {

        if (wall_obj == NULL) return(false);

        // the run's orientation, start, and color are required
        const cchar*  wall_orientation_cstr = NULL;
        const cchar*  wall_color_cstr       = NULL;
        u32           wall_row              = 0;
        u32           wall_col              = 0;
        map_color_u32 wall_color;

        const bool has_wall = (
            json_obj_try_get_cstr     (wall_obj, "orientation", wall_orientation_cstr) &&
            json_obj_try_get_u32      (wall_obj, "row",         wall_row)              &&
            json_obj_try_get_u32      (wall_obj, "col",         wall_col)              &&
            json_obj_try_get_cstr     (wall_obj, "color",       wall_color_cstr)       &&
            map_parse_color_from_cstr (wall_color_cstr,         wall_color)
        );
        if (!has_wall) return(false);

        map_wall_orientation_e wall_orientation;
        if      (strcmp(wall_orientation_cstr, "row")    == 0) wall_orientation = map_wall_orientation_e_row;
        else if (strcmp(wall_orientation_cstr, "column") == 0) wall_orientation = map_wall_orientation_e_column;
        else return(false);

        // the length is optional, but if it's there it has to be a positive number
        u32 wall_length = 1;
        if (json_obj_has_member(wall_obj, "length")) {
            if (!json_obj_try_get_u32(wall_obj, "length", wall_length)) return(false);
            if (wall_length == 0)                                        return(false);
        }

        // the sprite is optional, but needs both the atlas and the sprite name
        const bool   has_atlas   = json_obj_has_member(wall_obj, "atlas");
        const bool   has_sprite  = json_obj_has_member(wall_obj, "sprite");
        const cchar* wall_atlas  = NULL;
        const cchar* wall_sprite = NULL;
        if (has_atlas != has_sprite) return(false);
        if (has_sprite) {
            const bool is_valid = (
                json_obj_try_get_cstr(wall_obj, "atlas",  wall_atlas) &&
                json_obj_try_get_cstr(wall_obj, "sprite", wall_sprite)
            );
            if (!is_valid) return(false);
        }

        // row walls run along x (columns), column walls run along z (rows)
        const bool is_row = (wall_orientation == map_wall_orientation_e_row);
        for (
            u32 i_wall = 0;
                i_wall < wall_length;
              ++i_wall) {

            const u32 row = is_row ? wall_row : (wall_row + i_wall);
            const u32 col = is_row ? (wall_col + i_wall) : wall_col;

            if (!map_wall_set(h_map, row, col, wall_orientation, wall_color)) return(false);
            if (has_sprite && !map_wall_set_sprite(h_map, row, col, wall_orientation, wall_atlas, wall_sprite)) return(false);
        }

        return(true);
    }

    // builds a map from a map file, see docs/map-format.md
    // the document is parsed in the arena and reverted before returning, the map copies what it needs
    // on any failure, the partly built map is destroyed and the handle is invalid
    // a map's handle is the hash of its name, so name_override (or NULL for the file's name)
    // lets the same file be loaded twice, like the game's copy and the map builder's
    IFB_INTERNAL hnd_map
    map_parse_from_json(
        const cchar*    json_data,
        const u32       json_length,
        const hnd_arena h_arena,
        const cchar*    name_override) {

        assert(json_data   != NULL);
        assert(json_length != 0);
        assert(h_arena     != INVALID_HANDLE);

        static constexpr u32 MAP_FORMAT_VERSION   = 1;
        static constexpr u32 MAP_NAME_LENGTH_MAX  = 15; // the name buffer is 16 with the terminator

        // save our spot in the arena
        const u32 save = arena_save(h_arena);

        // create the document, it's NULL if the json is invalid or the root isn't an object
        const json_doc* doc = json_doc_create(h_arena, json_length, json_data);
        if (!doc) {
            arena_revert(h_arena, save);
            return(INVALID_HANDLE);
        }

        // the map's fields are at the top level of the file
        const json_obj* map_obj = json_doc_get_root(doc);
        assert(map_obj);

        // get the map properties
        u32             map_version     = 0;
        const cchar*    map_name_cstr   = NULL;
        u32             map_rows        = 0;
        u32             map_cols        = 0;
        const json_arr* map_chunk_array = json_obj_get_arr(map_obj, "chunks");

        // we need a minimum of these properties to create a map
        const bool can_create = (
            json_obj_try_get_u32  (map_obj, "version", map_version)   &&
            json_obj_try_get_cstr (map_obj, "name",    map_name_cstr) &&
            json_obj_try_get_u32  (map_obj, "rows",    map_rows)      &&
            json_obj_try_get_u32  (map_obj, "cols",    map_cols)      &&
            map_version     == MAP_FORMAT_VERSION &&
            map_rows        >  0                  &&
            map_cols        >  0                  &&
            map_chunk_array != NULL
        );
        if (!can_create) {
            arena_revert(h_arena, save);
            return(INVALID_HANDLE);
        }

        // the file's name and the override both have to fit the map's name buffer
        const cchar* map_create_name = (name_override != NULL) ? name_override : map_name_cstr;
        const u32    map_name_length = cstr_nvar_length(map_name_cstr,   MAP_NAME_LENGTH_MAX + 1);
        const u32    map_create_len  = cstr_nvar_length(map_create_name, MAP_NAME_LENGTH_MAX + 1);
        const bool   is_name_valid   = (
            map_name_length != 0 && map_name_length <= MAP_NAME_LENGTH_MAX &&
            map_create_len  != 0 && map_create_len  <= MAP_NAME_LENGTH_MAX
        );
        if (!is_name_valid) {
            arena_revert(h_arena, save);
            return(INVALID_HANDLE);
        }

        const hnd_map h_map = map_create(
            map_create_name,
            map_rows,
            map_cols
        );
        if (h_map == INVALID_HANDLE) {
            arena_revert(h_arena, save);
            return(INVALID_HANDLE);
        }

        bool did_parse = true;

        // the origin is optional, but if it's there it has to be [x, y, z]
        if (did_parse && json_obj_has_member(map_obj, "origin")) {
            const json_arr* map_origin_array = json_obj_get_arr(map_obj, "origin");
            vec3            map_origin;
            did_parse = (
                map_origin_array != NULL                     &&
                json_arr_get_count(map_origin_array) == 3    &&
                json_arr_try_get_f32(map_origin_array, 0, map_origin.x) &&
                json_arr_try_get_f32(map_origin_array, 1, map_origin.y) &&
                json_arr_try_get_f32(map_origin_array, 2, map_origin.z) &&
                map_set_origin(h_map, map_origin)
            );
        }

        // chunks come before walls, because walls have to be on the edge of a chunk tile
        // a chunk's index is its position in the array
        const u32 map_chunk_count = json_arr_get_count(map_chunk_array);
        for (
            u32 i_chunk = 0;
                did_parse && i_chunk < map_chunk_count;
              ++i_chunk) {

            const json_obj* chunk_obj = json_arr_get_obj(map_chunk_array, i_chunk);
            did_parse = map_parse_chunk_from_json(h_map, chunk_obj);
        }

        // walls are optional, but if they're there they have to be an array
        if (did_parse && json_obj_has_member(map_obj, "walls")) {

            const json_arr* map_wall_array = json_obj_get_arr(map_obj, "walls");
            did_parse = (map_wall_array != NULL);

            const u32 map_wall_count = did_parse ? json_arr_get_count(map_wall_array) : 0;
            for (
                u32 i_wall = 0;
                    did_parse && i_wall < map_wall_count;
                  ++i_wall) {

                const json_obj* wall_obj = json_arr_get_obj(map_wall_array, i_wall);
                did_parse = map_parse_wall_from_json(h_map, wall_obj);
            }
        }

        // don't leave a partly built map behind, another map could get its handle from the same name
        if (!did_parse) {
            map_destroy(h_map);
        }

        // the map doesn't point into the document, so we're done with it either way
        arena_revert(h_arena, save);

        return(did_parse ? h_map : INVALID_HANDLE);
    }

    //--------------------------------------------------------------------
    // WRITING MAP FILES
    //--------------------------------------------------------------------

    // the color names in a map file, in map_color_e order, see docs/map-format.md
    static const cchar* MAP_JSON_COLOR_NAMES[] = {
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

    // the most different sprites one chunk can use in a map file
    static constexpr u32 MAP_JSON_PALETTE_CAPACITY = 256;

    struct map_json_writer {
        cchar* buffer;
        u32    capacity;
        u32    length;
        bool   is_ok;
    };

    // appends to the text, and stops writing for good if it doesn't fit
    inline void
    map_json_write(
        map_json_writer& w,
        const cchar*     format,
        ...) {

        if (!w.is_ok) return;

        va_list args;
        va_start(args, format);
        const s32 count = vsnprintf(w.buffer + w.length, w.capacity - w.length, format, args);
        va_end(args);

        if (count < 0 || (u32)count >= (w.capacity - w.length)) {
            w.is_ok = false;
            return;
        }
        w.length += (u32)count;
    }

    inline const cchar*
    map_json_color_name(
        const u32 color) {

        const u32 count = sizeof(MAP_JSON_COLOR_NAMES) / sizeof(MAP_JSON_COLOR_NAMES[0]);
        return((color < count) ? MAP_JSON_COLOR_NAMES[color] : NULL);
    }

    // asset names fill all 16 characters, so they're copied somewhere with room for the terminator
    inline bool
    map_json_atlas_name(
        const hnd_sprite_atlas h_atlas,
        cchar*                 name) {

        name[0] = 0;

        const asset_database* db = asset_mngr_get_database();
        if (db == NULL || h_atlas == INVALID_HANDLE) return(false);

        // the atlas handle is the atlas's asset handle
        const asset_name* atlas_name = asset_database_get_name(db, hnd_asset(h_atlas.val));
        if (atlas_name == NULL) return(false);

        memcpy(name, atlas_name->cstr, config::asset_name_size);
        name[config::asset_name_size] = 0;
        return(name[0] != 0);
    }

    inline bool
    map_json_sprite_name(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        cchar*                 name) {

        name[0] = 0;

        sprite_info info;
        if (!sprite_mngr_get_sprite_info_by_index(h_atlas, i_sprite, info)) return(false);

        memcpy(name, info.name.cstr, config::asset_name_size);
        name[config::asset_name_size] = 0;
        return(name[0] != 0);
    }

    // one chunk of the chunks array
    // a chunk's sprites are listed most used first, so a chunk filled with one sprite has it at index 0
    inline bool
    map_json_write_chunk(
        map_json_writer& w,
        const map_chunk& chunk,
        const bool       is_last) {

        const cchar* color_name = map_json_color_name(chunk.base_color.val);
        if (color_name == NULL) return(false);

        // the sprites the chunk's tiles use, and how many tiles use each one
        u32 palette_sprite [MAP_JSON_PALETTE_CAPACITY];
        u32 palette_count  [MAP_JSON_PALETTE_CAPACITY];
        u32 palette_size = 0;

        const u32 tile_count = chunk.count_rows * chunk.count_cols;
        for (u32 i_tile = 0; i_tile < tile_count; ++i_tile) {

            const map_floor_tile& tile = chunk.floor[i_tile];
            const bool is_textured = (tile.flags & (u8)map_tile_flag_e_textured) != 0;
            if (!is_textured || tile.i_sprite == INVALID_INDEX) continue;

            u32 i_palette = 0;
            while (i_palette < palette_size && palette_sprite[i_palette] != tile.i_sprite) ++i_palette;
            if (i_palette == palette_size) {
                if (palette_size == MAP_JSON_PALETTE_CAPACITY) return(false);
                palette_sprite[palette_size] = tile.i_sprite;
                palette_count [palette_size] = 0;
                ++palette_size;
            }
            ++palette_count[i_palette];
        }

        // most used first, ties in atlas order, so saving the same map twice gives the same file
        for (u32 i = 1; i < palette_size; ++i) {
            const u32 sprite = palette_sprite[i];
            const u32 count  = palette_count[i];
            u32 j = i;
            while (j > 0 && (palette_count[j - 1] < count || (palette_count[j - 1] == count && palette_sprite[j - 1] > sprite))) {
                palette_sprite[j] = palette_sprite[j - 1];
                palette_count [j] = palette_count [j - 1];
                --j;
            }
            palette_sprite[j] = sprite;
            palette_count [j] = count;
        }

        map_json_write(w, "        {\n");
        map_json_write(w, "            \"row\": %u, \"col\": %u, \"rows\": %u, \"cols\": %u,\n",
            chunk.origin_row, chunk.origin_col, chunk.count_rows, chunk.count_cols);

        // a chunk with no sprites is a solid color
        if (palette_size == 0) {
            map_json_write(w, "            \"color\": \"%s\"\n", color_name);
        }
        else {
            cchar atlas_name[config::asset_name_size + 1];
            if (!map_json_atlas_name(chunk.h_atlas, atlas_name)) return(false);

            map_json_write(w, "            \"color\":   \"%s\",\n", color_name);
            map_json_write(w, "            \"atlas\":   \"%s\",\n", atlas_name);
            map_json_write(w, "            \"sprites\": [");
            for (u32 i_palette = 0; i_palette < palette_size; ++i_palette) {
                cchar sprite_name[config::asset_name_size + 1];
                if (!map_json_sprite_name(chunk.h_atlas, palette_sprite[i_palette], sprite_name)) return(false);
                map_json_write(w, (i_palette == 0) ? "\"%s\"" : ", \"%s\"", sprite_name);
            }
            map_json_write(w, "],\n");

            // one line per chunk row, each entry an index into sprites, or -1 for no sprite
            map_json_write(w, "            \"tiles\": [\n");
            for (u32 i_row = 0; i_row < chunk.count_rows; ++i_row) {
                map_json_write(w, "                ");
                for (u32 i_col = 0; i_col < chunk.count_cols; ++i_col) {

                    const map_floor_tile& tile = chunk.floor[(i_row * chunk.count_cols) + i_col];
                    const bool is_textured = (tile.flags & (u8)map_tile_flag_e_textured) != 0;

                    s32 entry = -1;
                    for (u32 i_palette = 0; is_textured && i_palette < palette_size; ++i_palette) {
                        if (palette_sprite[i_palette] == tile.i_sprite) { entry = (s32)i_palette; break; }
                    }

                    const bool is_last_tile = (i_row + 1 == chunk.count_rows) && (i_col + 1 == chunk.count_cols);
                    map_json_write(w, is_last_tile ? "%d" : ((i_col + 1 == chunk.count_cols) ? "%d," : "%d, "), entry);
                }
                map_json_write(w, "\n");
            }
            map_json_write(w, "            ]\n");
        }

        map_json_write(w, is_last ? "        }\n" : "        },\n");
        return(true);
    }

    // walls are written as runs, straight lines of walls next to each other that look the same
    // each wall is read from the chunk that owns its edge, which is the one map_wall_set writes to
    inline bool
    map_json_write_walls(
        map_json_writer&      w,
        const u32             map_index,
        const map_dimensions& dims) {

        bool is_first_run = true;

        for (u32 i_orientation = 0; i_orientation < 2; ++i_orientation) {

            const map_wall_orientation_e orientation = (i_orientation == 0)
                ? map_wall_orientation_e_row
                : map_wall_orientation_e_column;
            const bool   is_row           = (orientation == map_wall_orientation_e_row);
            const cchar* orientation_name = is_row ? "row" : "column";

            // row walls run along columns on the lines z = 0 to rows, column walls run along rows on x = 0 to cols
            const u32 line_count = is_row ? (dims.count_rows + 1) : (dims.count_cols + 1);
            const u32 step_count = is_row ?  dims.count_cols      :  dims.count_rows;

            for (u32 i_line = 0; i_line < line_count; ++i_line) {

                u32 i_step = 0;
                while (i_step < step_count) {

                    const s32 row = is_row ? (s32)i_line : (s32)i_step;
                    const s32 col = is_row ? (s32)i_step : (s32)i_line;

                    map_chunk*           chunk = NULL;
                    const map_wall_tile* wall  = map_find_wall(map_index, row, col, orientation, &chunk);
                    if (wall == NULL || (wall->flags & (u8)map_wall_flag_e_placed) == 0) {
                        ++i_step;
                        continue;
                    }

                    // the run goes on while the next wall is placed and looks the same
                    const hnd_sprite_atlas h_atlas = (wall->i_sprite != INVALID_INDEX) ? chunk->h_atlas : hnd_sprite_atlas(INVALID_HANDLE);
                    u32 length = 1;
                    while ((i_step + length) < step_count) {

                        const s32 next_row = is_row ? row : (row + (s32)length);
                        const s32 next_col = is_row ? (col + (s32)length) : col;

                        map_chunk*           next_chunk = NULL;
                        const map_wall_tile* next       = map_find_wall(map_index, next_row, next_col, orientation, &next_chunk);
                        if (next == NULL || (next->flags & (u8)map_wall_flag_e_placed) == 0) break;

                        const hnd_sprite_atlas next_atlas = (next->i_sprite != INVALID_INDEX) ? next_chunk->h_atlas : hnd_sprite_atlas(INVALID_HANDLE);
                        const bool is_same = (
                            next->base_color == wall->base_color &&
                            next->i_sprite   == wall->i_sprite   &&
                            next_atlas       == h_atlas
                        );
                        if (!is_same) break;
                        ++length;
                    }

                    const cchar* color_name = map_json_color_name(wall->base_color.val);
                    if (color_name == NULL) return(false);

                    map_json_write(w, is_first_run ? ",\n\n    \"walls\": [\n" : ",\n");
                    is_first_run = false;

                    map_json_write(w, "        { \"orientation\": \"%s\", \"row\": %d, \"col\": %d, \"length\": %u, \"color\": \"%s\"",
                        orientation_name, row, col, length, color_name);

                    if (wall->i_sprite != INVALID_INDEX) {
                        cchar atlas_name  [config::asset_name_size + 1];
                        cchar sprite_name [config::asset_name_size + 1];
                        if (!map_json_atlas_name  (h_atlas, atlas_name))                 return(false);
                        if (!map_json_sprite_name (h_atlas, wall->i_sprite, sprite_name)) return(false);
                        map_json_write(w, ", \"atlas\": \"%s\", \"sprite\": \"%s\"", atlas_name, sprite_name);
                    }
                    map_json_write(w, " }");

                    i_step += length;
                }
            }
        }

        // close the walls array, or the chunks array if there were no walls
        map_json_write(w, is_first_run ? "\n" : "\n    ]\n");
        return(true);
    }

    // writes a map as a map file, see docs/map-format.md, with map_name as its name
    // returns the length of the text, or 0 if the map can't be written or doesn't fit in the buffer
    IFB_INTERNAL u32
    map_write_json(
        const hnd_map h_map,
        const cchar*  map_name,
        cchar*        buffer,
        const u32     capacity) {

        assert(map_name != NULL);
        assert(buffer   != NULL);
        assert(capacity != 0);

        static constexpr u32 MAP_NAME_LENGTH_MAX = 15; // the name buffer is 16 with the terminator

        const u32 map_index = map_lookup_index(h_map);
        if (map_index == INVALID_INDEX) return(0);

        const u32 name_length = cstr_nvar_length(map_name, MAP_NAME_LENGTH_MAX + 1);
        if (name_length == 0 || name_length > MAP_NAME_LENGTH_MAX) return(0);

        // the name goes in a json string as it is, so it can't have characters that need escaping
        for (u32 i = 0; i < name_length; ++i) {
            if (map_name[i] == '"' || map_name[i] == '\\' || (u8)map_name[i] < 0x20) return(0);
        }

        const map_dimensions& dims   = map_get_dimensions(map_index);
        const vec3&           origin = _map_mngr->tbl_map->origin[map_index];

        map_json_writer w;
        w.buffer   = buffer;
        w.capacity = capacity;
        w.length   = 0;
        w.is_ok    = true;

        map_json_write(w, "{\n");
        map_json_write(w, "    \"version\": 1,\n");
        map_json_write(w, "    \"name\":    \"%s\",\n", map_name);
        map_json_write(w, "    \"rows\":    %u,\n", dims.count_rows);
        map_json_write(w, "    \"cols\":    %u,\n", dims.count_cols);

        // the origin is optional, so it's only written when it isn't the default
        // 9 significant digits gives back the same f32
        if (origin.x != 0.0f || origin.y != 0.0f || origin.z != 0.0f) {
            map_json_write(w, "    \"origin\":  [%.9g, %.9g, %.9g],\n", origin.x, origin.y, origin.z);
        }

        map_json_write(w, "\n    \"chunks\": [\n");
        for (u32 i_chunk = 0; i_chunk < dims.count_chunks; ++i_chunk) {
            const map_chunk& chunk = map_get_chunk(map_index, i_chunk);
            if (!map_json_write_chunk(w, chunk, (i_chunk + 1) == dims.count_chunks)) return(0);
        }
        map_json_write(w, "    ]");

        if (!map_json_write_walls(w, map_index, dims)) return(0);
        map_json_write(w, "}\n");

        return(w.is_ok ? w.length : 0);
    }

    //--------------------------------------------------------------------
    // INTERNAL METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL u32
    map_lookup_index(
        const hnd_map map_hnd) {

        assert(_map_mngr);
        const map_table* tbl_map = _map_mngr->tbl_map;
        assert(tbl_map);

        if (map_hnd == INVALID_HANDLE) {
            return(INVALID_INDEX);
        }

        const auto& cfg = config_instance();

        for (
            u32 index_curr = 0;
                index_curr < cfg.map_capacity;
              ++index_curr) {

            if (tbl_map->hnd[index_curr] == map_hnd) {
                return(index_curr);
            }
        }

        return(INVALID_INDEX);
    }

    IFB_INTERNAL map_chunk&
    map_get_chunk(
        const u32 map_index,
        const u32 chunk_index) {

        assert(_map_mngr);
        map_table* map_tbl = _map_mngr->tbl_map;
        assert(map_tbl != NULL);
        map_chunk_array* chunk_arrays = map_tbl->chunk_array;
        assert(chunk_arrays);

        const auto& cfg = config_instance();

        assert(chunk_index < cfg.map_chunk_capacity);
        assert(map_tbl->hnd[map_index] != INVALID_HANDLE);

        map_chunk_array& chunk_array = chunk_arrays      [map_index];
        map_chunk&       chunk       = chunk_array.chunks[chunk_index];

        return(chunk);
    }

    IFB_INTERNAL map_floor_array&
    map_get_floor_array(
        const u32 map_index) {

        assert(_map_mngr);
        map_table* map_tbl = _map_mngr->tbl_map;
        assert(map_tbl != NULL);
        assert(map_tbl->floor_array);

        const auto& cfg = config_instance();

        assert(map_index < cfg.map_capacity);
        assert(map_tbl->hnd[map_index] != INVALID_HANDLE);

        return(map_tbl->floor_array[map_index]);
    }

    IFB_INTERNAL map_floor_tile*
    map_find_floor_tile(
        const u32   map_index,
        const s32   row,
        const s32   col,
        map_chunk** chunk_out) {

        if (row < 0 || col < 0) {
            return(NULL);
        }

        // find the chunk that covers the tile
        const map_dimensions& dims = map_get_dimensions(map_index);
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            map_chunk& chunk = map_get_chunk(map_index, chunk_index);

            const bool is_row_in_chunk = ((u32)row >= chunk.origin_row && (u32)row < (chunk.origin_row + chunk.count_rows));
            const bool is_col_in_chunk = ((u32)col >= chunk.origin_col && (u32)col < (chunk.origin_col + chunk.count_cols));
            if (is_row_in_chunk && is_col_in_chunk) {

                const u32 local_row = (u32)row - chunk.origin_row;
                const u32 local_col = (u32)col - chunk.origin_col;
                if (chunk_out != NULL) {
                    *chunk_out = &chunk;
                }
                return(&chunk.floor[(local_row * chunk.count_cols) + local_col]);
            }
        }

        return(NULL);
    }

    IFB_INTERNAL bool
    map_get_sprite_rect(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        f32&                   u_left,
        f32&                   v_top,
        f32&                   u_right,
        f32&                   v_bottom) {

        // the sprite's rect in the atlas, normalized
        // the atlas isn't flipped on upload, so the top row is v = 0
        // with no sprite, it's the whole texture and the caller draws a solid color
        u_left   = 0.0f;
        u_right  = 1.0f;
        v_top    = 0.0f;
        v_bottom = 1.0f;

        sprite_info info;
        const bool did_get_sprite_info = (
            h_atlas  != INVALID_HANDLE &&
            i_sprite != INVALID_INDEX  &&
            sprite_mngr_get_sprite_info_by_index(h_atlas, i_sprite, info)
        );
        if (did_get_sprite_info) {
            const f32 atlas_width  = (f32)info.atlas_width;
            const f32 atlas_height = (f32)info.atlas_height;
            u_left   = (f32)(info.pixel_x)                     / atlas_width;
            u_right  = (f32)(info.pixel_x + info.pixel_width)  / atlas_width;
            v_top    = (f32)(info.pixel_y)                     / atlas_height;
            v_bottom = (f32)(info.pixel_y + info.pixel_height) / atlas_height;
        }
        return(did_get_sprite_info);
    }

    IFB_INTERNAL map_wall_array&
    map_get_wall_array(
        const u32                    map_index,
        const map_wall_orientation_e orientation) {

        assert(_map_mngr);
        map_table* map_tbl = _map_mngr->tbl_map;
        assert(map_tbl != NULL);
        assert(map_tbl->row_wall_array);
        assert(map_tbl->column_wall_array);

        const auto& cfg = config_instance();

        assert(map_index < cfg.map_capacity);
        assert(map_tbl->hnd[map_index] != INVALID_HANDLE);

        map_wall_array* wall_arrays = (orientation == map_wall_orientation_e_row)
            ? map_tbl->row_wall_array
            : map_tbl->column_wall_array;
        return(wall_arrays[map_index]);
    }

    IFB_INTERNAL map_wall_tile*
    map_chunk_get_wall(
        map_chunk&                   chunk,
        const s32                    row,
        const s32                    col,
        const map_wall_orientation_e orientation) {

        if (row < 0 || col < 0) {
            return(NULL);
        }

        // walls run along the edges, so the chunk has one extra line of them
        // on the far side in the direction they're stacked
        const bool is_row      = (orientation == map_wall_orientation_e_row);
        const u32  wall_rows   = is_row ? (chunk.count_rows + 1) : chunk.count_rows;
        const u32  wall_cols   = is_row ? chunk.count_cols       : (chunk.count_cols + 1);

        const bool is_row_in_chunk = ((u32)row >= chunk.origin_row && (u32)row < (chunk.origin_row + wall_rows));
        const bool is_col_in_chunk = ((u32)col >= chunk.origin_col && (u32)col < (chunk.origin_col + wall_cols));
        if (!is_row_in_chunk || !is_col_in_chunk) {
            return(NULL);
        }

        const u32      local_row = (u32)row - chunk.origin_row;
        const u32      local_col = (u32)col - chunk.origin_col;
        map_wall_tile* walls     = is_row ? chunk.row_walls : chunk.column_walls;
        assert(walls);
        return(&walls[(local_row * wall_cols) + local_col]);
    }

    IFB_INTERNAL map_wall_tile*
    map_find_wall(
        const u32                    map_index,
        const s32                    row,
        const s32                    col,
        const map_wall_orientation_e orientation,
        map_chunk**                  chunk_out) {

        // an edge between two chunks is in both of them,
        // the first chunk that has it owns it for setting
        const map_dimensions& dims = map_get_dimensions(map_index);
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            map_chunk&     chunk = map_get_chunk(map_index, chunk_index);
            map_wall_tile* wall  = map_chunk_get_wall(chunk, row, col, orientation);
            if (wall != NULL) {
                if (chunk_out != NULL) {
                    *chunk_out = &chunk;
                }
                return(wall);
            }
        }

        return(NULL);
    }

    IFB_INTERNAL bool
    map_has_wall(
        const u32                    map_index,
        const s32                    row,
        const s32                    col,
        const map_wall_orientation_e orientation) {

        // check every chunk with the edge, not just the owner
        const map_dimensions& dims = map_get_dimensions(map_index);
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            map_chunk&           chunk = map_get_chunk(map_index, chunk_index);
            const map_wall_tile* wall  = map_chunk_get_wall(chunk, row, col, orientation);
            if (wall != NULL && (wall->flags & (u8)map_wall_flag_e_placed) != 0) {
                return(true);
            }
        }

        return(false);
    }
    
    
    IFB_INTERNAL bool
    map_is_tile_navigable(
        const u32 map_index,
        const s32 row,
        const s32 col) {

        // a tile is navigable if a chunk covers it and it's flagged navigable
        const map_floor_tile* floor_tile = map_find_floor_tile(map_index, row, col, NULL);
        const bool is_navigable = (
            floor_tile != NULL &&
            (floor_tile->flags & (u8)map_tile_flag_e_navigable) != 0
        );
        return(is_navigable);
    }

    IFB_INTERNAL bool
    map_get_dimensions(
        const hnd_map   map_hnd,
        map_dimensions& dims) {
      
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }
        
        dims = map_get_dimensions(map_index);
        return(true);
    }
};
