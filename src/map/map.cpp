#pragma once

#include "component-tables.cpp"
#include "ifb-collections.hpp"
#include "ifb-config.hpp"
#include "map-internal.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "sld.hpp"
#include "map.hpp"
#include "entity.hpp"
#include <cassert>
#include <cmath>
#include <sld-strings.hpp>

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
        vec3&            origin = tbl_map->origin      [index];

        cstr_c16_init(&name, map_name);
        hnd = cstr_c16_hash(&name);
        dims.count_chunks = 0;
        dims.count_rows   = count_rows;
        dims.count_cols   = count_cols;
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
    
    IFB_INTERNAL bool
    map_render(
        const hnd_map map_hnd) {
       
        // validate
        assert(map_hnd            != INVALID_HANDLE);
        assert(_map_mngr          != NULL);
        assert(_map_mngr->tbl_map != NULL);

        // get the map index
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }

        // get the map dimensions
        const map_dimensions& dims = map_get_dimensions(map_index);

        // count the tiles
        u32 tile_count = 0;
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk = map_get_chunk(map_index, chunk_index);
            tile_count += (chunk.count_rows * chunk.count_cols);
        }
        if (tile_count == 0) {
            return(false);
        }

        // calculate data size buffer
        // and make sure the buffer is large enough
        const auto& cfg     = config_instance();
        const u32 size_data = sizeof(map_tile) * tile_count; 
        assert(size_data <= cfg.map_render_buffer_size);

        // initialize buffer
        map_render_buffer& buffer = _map_mngr->render_buffer;
        buffer.data_size          = size_data;

        // fill out the render buffer
        u32 tile_index = 0;
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk            = map_get_chunk(map_index, chunk_index);
            const u32        chunk_tile_count = chunk.count_rows * chunk.count_cols;
            for (
                u32 chunk_tile_index = 0;
                    chunk_tile_index < chunk_tile_count;
                  ++chunk_tile_index) {

                // calculate the row and column
                const u32 row = (chunk_tile_index / chunk.count_cols) + chunk.origin_row;
                const u32 col = (chunk_tile_index % chunk.count_cols) + chunk.origin_col;

                // get the next tile
                map_tile& tile = buffer.data.tile_array[tile_index++]; 

                // initialize the tile
                tile.index = (row * dims.count_cols) + col;
                tile.color = (u8)chunk.base_color.val;
            }
        }
        return(true);
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

        // push the box out of any tiles that aren't on a chunk
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

            // find the tile the box overlaps the most,
            // resolving that one first keeps the box from
            // catching on the seams between tiles along an edge
            f32 overlap_area_max = 0.0f;
            f32 tile_push_x      = 0.0f;
            f32 tile_push_z      = 0.0f;
            for (s32 row = row_min; row <= row_max; ++row) {
                for (s32 col = col_min; col <= col_max; ++col) {

                    if (map_is_tile_navigable(map_index, row, col)) continue;

                    const f32 tile_x_min = (f32)col;
                    const f32 tile_z_min = (f32)row;
                    const f32 tile_x_max = (f32)(col + 1);
                    const f32 tile_z_max = (f32)(row + 1);

                    const f32 overlap_x = ((box_x_max < tile_x_max) ? box_x_max : tile_x_max) - ((box_x_min > tile_x_min) ? box_x_min : tile_x_min);
                    const f32 overlap_z = ((box_z_max < tile_z_max) ? box_z_max : tile_z_max) - ((box_z_min > tile_z_min) ? box_z_min : tile_z_min);
                    if (overlap_x <= MAP_CONSTRAIN_EPSILON || overlap_z <= MAP_CONSTRAIN_EPSILON) continue;

                    const f32 overlap_area = overlap_x * overlap_z;
                    if (overlap_area <= overlap_area_max) continue;
                    overlap_area_max = overlap_area;

                    // the shortest way out of the tile
                    const f32 exit_x = ((box_x_max - tile_x_min) < (tile_x_max - box_x_min)) ? (tile_x_min - box_x_max) : (tile_x_max - box_x_min);
                    const f32 exit_z = ((box_z_max - tile_z_min) < (tile_z_max - box_z_min)) ? (tile_z_min - box_z_max) : (tile_z_max - box_z_min);
                    const bool is_exit_x = (fabsf(exit_x) < fabsf(exit_z));
                    tile_push_x = is_exit_x ? exit_x : 0.0f;
                    tile_push_z = is_exit_x ? 0.0f   : exit_z;
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
        entity_list* e_list = entity_list_arena_create(h_arena);
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
                    (void)entity_list_add(e_list, e.id);     
                }    
            } 
        }

        return(e_list);
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
    
    
    IFB_INTERNAL bool
    map_is_tile_navigable(
        const u32 map_index,
        const s32 row,
        const s32 col) {

        if (row < 0 || col < 0) {
            return(false);
        }

        // a tile is navigable if any chunk covers it
        const map_dimensions& dims = map_get_dimensions(map_index);
        for (
            u32 chunk_index = 0;
                chunk_index < dims.count_chunks;
              ++chunk_index) {

            const map_chunk& chunk = map_get_chunk(map_index, chunk_index);

            const bool is_row_in_chunk = ((u32)row >= chunk.origin_row && (u32)row < (chunk.origin_row + chunk.count_rows));
            const bool is_col_in_chunk = ((u32)col >= chunk.origin_col && (u32)col < (chunk.origin_col + chunk.count_cols));
            if (is_row_in_chunk && is_col_in_chunk) {
                return(true);
            }
        }

        return(false);
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
