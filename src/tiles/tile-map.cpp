#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "sld.hpp"
#include "tile.hpp"
#include <cassert>
#include <sld-strings.hpp>

namespace ifb {
   
    struct tile_id_composite {
        union {
            struct {
                u16 row;
                u16 col;
            };
            u32 id;
        };
    };

    inline u32
    tile_map_name_hash(
        tile_map_name& name) {

        const u32 hash = hash_u32((void*)name.cstr, sizeof(name.cstr));
        return(hash); 
    }

    inline u32
    tile_map_index(
        const tile_map_table* map_tbl,
        const tile_map_id_u32 map_id,
        const u32             map_count) {
    
        assert(map_tbl   != NULL); 
        assert(map_id    != INVALID_ID); 
        assert(map_count != 0); 

        u32 result = INVALID_INDEX;
        for (
            u32 index = 0;
            index < map_count;
            ++index) {

            if (map_tbl->map_id[index] == map_id) {
                result = index;
                break;
            }
        }

        return(result);
    }

    inline u32
    tile_get_index(
        const tile_table* tbl_tile,
        const u32         tile_row,
        const u32         tile_col,
        const u32         map_count_row,
        const u32         map_count_col,
        const u32         offset) {
    
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

    IFB_INTERNAL tile_map_id_u32
    tile_map_create(
        const cchar*         name,
        const f32            tile_width,
        const f32            tile_height,
        const u32            count_rows,
        const u32            count_col,
        const color_rgba_u32 base_color
        ) {

        assert(_tile_mngr  != NULL);
        assert(name        != NULL);
        assert(tile_width  != 0.0f);
        assert(tile_height != 0.0f);
        assert(count_rows  != 0);
        assert(count_col   != 0);

        // cache tables
        auto tbl_map  = _tile_mngr->tbl_map;
        auto tbl_tile = _tile_mngr->tbl_tiles;


        // find a free spot in the map id array of the tile table
        u32  map_index = INVALID_INDEX;
        for (
            u32 i = 0;
            i < _tile_mngr->map_capacity; 
            ++i) {

            if (tbl_map->map_id[i] == INVALID_ID) {
                map_index = i;
                break;
            }
        }
        if (map_index == INVALID_INDEX) {
            return(INVALID_ID);
        }

        // calculate the offset in the tile table
        const u32 offset = _tile_mngr->tiles_per_map * map_index; 

        // store the name and calculate hash
        const u32      name_len      = cstr_nvar_length(name, 16);
        tile_map_name& map_name      = tbl_map->name[map_index];
        const u32      length_copied = cstr_nvar_copy(
            name,
            name_len,
            map_name.cstr,
            16
        );
        const u32 map_id = tile_map_name_hash(map_name);
        assert(map_id != INVALID_ID);

        // write the values to the table
        tbl_map->map_id      [map_index] = map_id;
        tbl_map->tile_width  [map_index] = tile_width;
        tbl_map->tile_height [map_index] = tile_height;
        tbl_map->count_rows  [map_index] = count_rows;
        tbl_map->count_cols  [map_index] = count_col;

        // write the base color to the table
        const u32 tile_count = count_rows * count_col; 
        for (
            u32 tile_index = 0;
                tile_index < tile_count;
              ++tile_index) {

            tbl_tile->color[tile_index] = base_color;
        }

        //return the id
        return(map_id); 
    }

    IFB_INTERNAL void
    tile_map_destroy(
        const tile_map_id_u32 map_id) {

        //TODO
    }

    IFB_INTERNAL u32
    tile_map_tile_count(
        const tile_map_id_u32 map_id) {


        assert(map_id != INVALID_HASH_32);

        auto tbl = _tile_mngr->tbl_map;
        assert(tbl != NULL);

        // get the index for the map
        const u32 index = tile_map_index(tbl, map_id, _tile_mngr->map_capacity);
        assert(index != INVALID_INDEX);

        const u32 rows  = tbl->count_rows[index];
        const u32 cols  = tbl->count_cols[index];
        const u32 count = rows * cols;

        return(count);
    }

    IFB_INTERNAL void
    tile_map_set_color(
        const tile_map_id_u32 map_id,
        const tile_coords*    coords,
        const color_rgba_u32* color,
        const u32             count) {

        assert(map_id     != INVALID_HASH_32);
        assert(coords     != NULL);
        assert(color      != NULL);
        assert(count      != 0);
        assert(_tile_mngr != NULL); 
      
        auto        tbl_map  = _tile_mngr->tbl_map;
        auto        tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = tile_map_index    (tbl_map, map_id, _tile_mngr->map_capacity);
        const u32 offset     = tile_table_offset (index, _tile_mngr->tiles_per_map);
        const u32 count_rows = tbl_map->count_rows[index];                
        const u32 count_cols = tbl_map->count_cols[index];

        for (
            u32 i = 0;
            i < count;
            ++i) {
      
            const tile_coords& curr_coords = coords[i];
            assert(curr_coords.row < count_rows);
            assert(curr_coords.col < count_cols);

            const u32 tile_index = ((count_rows * curr_coords.row) + curr_coords.col) + offset;

            tbl_tile->color[tile_index] = color[i];
        }
    }

    IFB_INTERNAL void
    tile_map_set_flags(
        const tile_map_id_u32 map_id,
        const tile_coords*    coords,
        const tile_flags_u32* flags,
        const u32             count) {

        assert(map_id     != INVALID_HASH_32);
        assert(coords     != NULL);
        assert(flags      != NULL);
        assert(count      != 0);
        assert(_tile_mngr != NULL); 
       
        auto tbl_map  = _tile_mngr->tbl_map;
        auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = tile_map_index    (tbl_map, map_id, _tile_mngr->map_capacity);
        const u32 offset     = tile_table_offset (index,   _tile_mngr->tiles_per_map);
        const u32 count_rows = tbl_map->count_rows[index];                
        const u32 count_cols = tbl_map->count_cols[index];

        for (
            u32 i = 0;
            i < count;
            ++i) {
      
            const tile_coords& curr_coords = coords[i];
            assert(curr_coords.row < count_rows);
            assert(curr_coords.col < count_cols);

            const u32 tile_index = ((count_rows * curr_coords.row) + curr_coords.col) + offset;

            tbl_tile->flags[tile_index] = flags[i];
        }
    }

    IFB_INTERNAL u32
    tile_map_get_render_buffer_size(
        const tile_map_id_u32 map_id) {

        const auto tbl_map  = _tile_mngr->tbl_map;
        const auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        // get the map index
        const u32 map_index = tile_map_index(tbl_map, map_id, _tile_mngr->map_capacity);  
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
    tile_map_get_render_buffer_data(
        const tile_map_id_u32 map_id,
        const u32             buffer_size,
        byte*                 buffer_data) {

        assert(map_id      != INVALID_ID);
        assert(buffer_size != 0);
        assert(buffer_data != NULL);

        const auto tbl_map  = _tile_mngr->tbl_map;
        const auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        // get the map index
        const u32 map_index = tile_map_index(tbl_map, map_id, _tile_mngr->map_capacity);  
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
    tile_map_get_info(
        const tile_map_id_u32 map_id,
        tile_map&             map) {

        assert(map_id != INVALID_ID);
        assert(_tile_mngr);

        auto tbl_map = _tile_mngr->tbl_map;
        assert(tbl_map != NULL);

        bool found = false;
        for (
            u32 i = 0;
            i < _tile_mngr->map_capacity;
            ++i) {

            if (map_id == tbl_map->map_id[i]) {
               
                map.id          =  map_id;
                map.tile_width  =  tbl_map->tile_width  [i];
                map.tile_height =  tbl_map->tile_height [i];
                map.count_rows  =  tbl_map->count_rows  [i];
                map.count_cols  =  tbl_map->count_cols  [i];
                map.name        = &tbl_map->name        [i];
                
                found = true;
                break;
            } 
        }

        return(found);
    }
};
