#pragma once

#include "component-tables.cpp"
#include "entity.cpp"
#include "ifb-types.hpp"
#include "sld.hpp"
#include "map.hpp"
#include <cassert>
#include <sld-strings.hpp>

namespace ifb {
   
    inline u32
    map_name_hash(
        map_name& name) {

        const u32 hash = hash_u32((void*)name.cstr, sizeof(name.cstr));
        return(hash); 
    }

    inline u32
    map_get_index(
        const map_table* map_tbl,
        const map_handle map_hnd,
        const u32        map_count) {
    
        assert(map_tbl   != NULL); 
        assert(map_hnd    != INVALID_ID); 
        assert(map_count != 0); 

        u32 result = INVALID_INDEX;
        for (
            u32 index = 0;
            index < map_count;
            ++index) {

            if (map_tbl->map_hnd[index] == map_hnd) {
                result = index;
                break;
            }
        }

        return(result);
    }

    inline u32
    tile_get_index(
        const map_tile_table* tbl_tile,
        const u32             tile_row,
        const u32             tile_col,
        const u32             map_count_row,
        const u32             map_count_col,
        const u32             offset) {
    
        bool is_valid = true;
        is_valid &= tile_row < map_count_row;
        is_valid &= tile_col < map_count_col;

        const u32 index = is_valid 
            ? offset + tile_col + (tile_row * map_count_row)
            : INVALID_INDEX;

        return(index);
    }

    inline u32
    tile_table_offset(
        const u32  map_index,
        const u32  map_tile_capacity) {
    
        const u32 offset = map_index * map_tile_capacity;

        return(offset);
    }

    IFB_INTERNAL map_handle
    map_create(
        const cchar*         name,
        const u32            count_rows,
        const u32            count_col,
        const s32            offset_row,
        const s32            offset_col,
        const color_rgba_u32 base_color) {

        assert(_map_mngr  != NULL);
        assert(name       != NULL);
        assert(count_rows != 0);
        assert(count_col  != 0);

        // cache tables
        auto tbl_map  = _map_mngr->tbl_map;
        auto tbl_tile = _map_mngr->tbl_tiles;

        // find a free spot in the map id array of the tile table
        u32 map_index = INVALID_INDEX;
        for (
            u32 i = 0;
            i < _map_mngr->map_capacity; 
            ++i) {

            if (tbl_map->map_hnd[i] == INVALID_ID) {
                map_index = i;
                break;
            }
        }
        if (map_index == INVALID_INDEX) {
            return(INVALID_ID);
        }

        // calculate the offset in the tile table
        const u32 offset = _map_mngr->tiles_per_map * map_index; 

        // store the name and calculate hash
        const u32 name_len      = cstr_nvar_length(name, 16);
        map_name& map_name      = tbl_map->name[map_index];
        const u32 length_copied = cstr_nvar_copy(
            name,
            name_len,
            map_name.cstr,
            16
        );
        const u32 map_hnd = map_name_hash(map_name);
        assert(map_hnd != INVALID_ID);

        // write the values to the table
        tbl_map->map_hnd     [map_index] = map_hnd;
        tbl_map->count_rows [map_index] = count_rows;
        tbl_map->count_cols [map_index] = count_col;
        tbl_map->offset_row [map_index] = offset_row;
        tbl_map->offset_col [map_index] = offset_col;

        // write the base color to the table
        const u32 tile_count = count_rows * count_col; 
        for (
            u32 tile_index = 0;
                tile_index < tile_count;
              ++tile_index) {

            tbl_tile->color[tile_index] = base_color;
        }

        //return the id
        return(map_hnd); 
    }

    IFB_INTERNAL void
    map_destroy(
        const map_handle map_hnd) {

        //TODO
    }

    IFB_INTERNAL u32
    map_tile_count(
        const map_handle map_hnd) {


        assert(map_hnd != INVALID_HASH_32);

        auto tbl = _map_mngr->tbl_map;
        assert(tbl != NULL);

        // get the index for the map
        const u32 index = map_get_index(tbl, map_hnd, _map_mngr->map_capacity);
        assert(index != INVALID_INDEX);

        const u32 rows  = tbl->count_rows[index];
        const u32 cols  = tbl->count_cols[index];
        const u32 count = rows * cols;

        return(count);
    }

    IFB_INTERNAL void
    map_set_color(
        const map_handle map_hnd,
        const map_coords*    coords,
        const color_rgba_u32* color,
        const u32             count) {

        assert(map_hnd     != INVALID_HASH_32);
        assert(coords     != NULL);
        assert(color      != NULL);
        assert(count      != 0);
        assert(_map_mngr != NULL); 
      
        auto        tbl_map  = _map_mngr->tbl_map;
        auto        tbl_tile = _map_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = map_get_index     (tbl_map, map_hnd, _map_mngr->map_capacity);
        const u32 offset     = tile_table_offset (index, _map_mngr->tiles_per_map);
        const u32 count_rows = tbl_map->count_rows[index];                
        const u32 count_cols = tbl_map->count_cols[index];

        for (
            u32 i = 0;
            i < count;
            ++i) {
      
            const map_coords& curr_coords = coords[i];
            assert(curr_coords.row < count_rows);
            assert(curr_coords.col < count_cols);

            const u32 tile_index = ((count_rows * curr_coords.row) + curr_coords.col) + offset;

            tbl_tile->color[tile_index] = color[i];
        }
    }

    IFB_INTERNAL void
    map_set_flags(
        const map_handle map_hnd,
        const map_coords*    coords,
        const tile_flags_u32* flags,
        const u32             count) {

        assert(map_hnd     != INVALID_HASH_32);
        assert(coords     != NULL);
        assert(flags      != NULL);
        assert(count      != 0);
        assert(_map_mngr != NULL); 
       
        auto tbl_map  = _map_mngr->tbl_map;
        auto tbl_tile = _map_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = map_get_index     (tbl_map, map_hnd, _map_mngr->map_capacity);
        const u32 offset     = tile_table_offset (index,   _map_mngr->tiles_per_map);
        const u32 count_rows = tbl_map->count_rows[index];                
        const u32 count_cols = tbl_map->count_cols[index];
                                
        for (
            u32 i = 0;
            i < count;
            ++i) {
      
            const map_coords& curr_coords = coords[i];
            assert(curr_coords.row < count_rows);
            assert(curr_coords.col < count_cols);

            const u32 tile_index = ((count_rows * curr_coords.row) + curr_coords.col) + offset;

            tbl_tile->flags[tile_index] = flags[i];
        }
    }

    IFB_INTERNAL u32
    map_get_render_buffer_size(
        const map_handle map_hnd) {

        const auto tbl_map  = _map_mngr->tbl_map;
        const auto tbl_tile = _map_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        // get the map index
        const u32 map_index = map_get_index(tbl_map, map_hnd, _map_mngr->map_capacity);  
        assert(map_index != INVALID_HASH_32);

        // get row and column count
        const u32 count_rows  = tbl_map->count_rows[map_index];  
        const u32 count_cols  = tbl_map->count_cols[map_index];  
        assert(count_rows != 0);
        assert(count_cols != 0);

        const u32 tile_count  = count_rows * count_cols;
        const u32 buffer_size = sizeof(tile_render_context) * tile_count;

        return(buffer_size);
    }
   
    IFB_INTERNAL u32 
    map_get_render_buffer_data(
        const map_handle map_hnd,
        const u32             buffer_size,
        byte*                 buffer_data) {

        assert(map_hnd      != INVALID_ID);
        assert(buffer_size != 0);
        assert(buffer_data != NULL);

        const auto tbl_map  = _map_mngr->tbl_map;
        const auto tbl_tile = _map_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        // get the map index
        const u32 map_index = map_get_index(tbl_map, map_hnd, _map_mngr->map_capacity);  
        assert(map_index != INVALID_HASH_32);

        // get row and column count
        const u32 count_rows  = tbl_map->count_rows[map_index];  
        const u32 count_cols  = tbl_map->count_cols[map_index];  
        assert(count_rows != 0);
        assert(count_cols != 0);

        const u32 tile_count = count_rows * count_cols;
        const u32 size_min   = tile_count * sizeof(tile_render_context);
        assert(buffer_size >= size_min);

        auto ctx_buff = (tile_render_context*)buffer_data;

        for (
            u32 i = 0;
            i < tile_count;
            ++i) {

            auto& ctx = ctx_buff[i];
            ctx.color = tbl_tile->color[i].hex;
        }

        // return the actual size we copied
        return(size_min);
    }

    IFB_INTERNAL bool
    map_get_info(
        const map_handle map_hnd,
        map&             map) {

        assert(map_hnd != INVALID_ID);
        assert(_map_mngr);

        auto tbl_map = _map_mngr->tbl_map;
        assert(tbl_map != NULL);

        bool found = false;
        for (
            u32 i = 0;
            i < _map_mngr->map_capacity;
            ++i) {

            if (map_hnd == tbl_map->map_hnd[i]) {
        
                // get table records
                map.id         =  map_hnd;
                map.count_rows =  tbl_map->count_rows [i];
                map.count_cols =  tbl_map->count_cols [i];
                map.offset_row =  tbl_map->offset_row [i];
                map.offset_col =  tbl_map->offset_col [i];
                map.name       = &tbl_map->name       [i];
               
                // calculate origin
                map.origin_x = _map_mngr->tile_unit_size * map.offset_col;
                map.origin_z = _map_mngr->tile_unit_size * map.offset_row;

                found = true;
                break;
            } 
        }

        return(found);
    }
    
    IFB_INTERNAL bool
    map_get_world_position(
        const map_handle map_hnd,
        const u32        row,
        const u32        col,
        position_3d&     pos) {

        map m;
        const bool found = map_get_info(map_hnd, m);
        if (found) {

            assert(row < m.count_rows);
            assert(col < m.count_cols);
            
            pos = {0};

            const s32 row_adj = (s32)row + m.offset_row;
            const s32 col_adj = (s32)col + m.offset_col;

            pos.x = col * _map_mngr->tile_unit_size; 
            pos.y = 0; 
            pos.z = row * _map_mngr->tile_unit_size; 
        }
        return(found);
    }

    IFB_INTERNAL bool
    map_get_tile_coordinates(
        const map_handle   map_hnd,
        const position_3d& pos,
        map_coords&       coords) {

        map m;
        const bool found = map_get_info(map_hnd, m);  
        if (found) {
   
            // calculate bounds
            const f32 map_width  = _map_mngr->tile_unit_size * m.count_cols;
            const f32 map_height = _map_mngr->tile_unit_size * m.count_rows;
            const f32 max_x      = m.origin_x + map_width;
            const f32 max_z      = m.origin_z + map_height;

            // determine if the position is inside the map
            bool is_valid = true;
            is_valid &= pos.x >= m.origin_x;
            is_valid &= pos.x <= max_x;
            is_valid &= pos.z >= m.origin_z;
            is_valid &= pos.z <= max_z; 
            if (!is_valid) {
                coords.row = MAP_COORD_INVALID;
                coords.col = MAP_COORD_INVALID;
                return(false);
            }

            const f32 local_x = pos.x - m.origin_x;
            const f32 local_z = pos.z - m.origin_z;

            coords.col = (u32)(local_x / _map_mngr->tile_unit_size);
            coords.row = (u32)(local_z / _map_mngr->tile_unit_size);
        }
        return(found);
    }

    IFB_INTERNAL bool
    map_get_entity_tile_coordinates(
        const map_handle map_hnd,
        const entity_id  e,
        map_coords&     coords) {

        // get the sparse index
        const u32 sparse_index = entity_lookup_sparse_index(e);  
        if (sparse_index == INVALID_INDEX) return(false); 

        // get the position
        position_3d pos;
        cmpnt_lookup_position(sparse_index, pos);

        // get the coordinates
        const bool did_get = map_get_tile_coordinates(map_hnd, pos, coords);
        return(did_get);
    } 
};
