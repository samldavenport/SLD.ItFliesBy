#pragma once

#include "files.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "tile.hpp"
#include <cassert>

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
        const tile_map_table* tile_map_tbl,
        const tile_map_id_u32 map_id) {
    
        assert(tile_map_tbl); 

        u32 result = INVALID_INDEX;
        for (
            u32 index = 0;
            index < tile_map_tbl->count;
            ++index) {

            if (tile_map_tbl->data.map_id[index] == map_id) {
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
        const tile_table*     tile_tbl,
        const tile_map_id_u32 map_id) {

        const auto cfg = config_instance();

        assert(map_id   != INVALID_HASH_32); 
        assert(tile_tbl != NULL);
    
        u32 result = INVALID_INDEX;

        for (
            u32 index = 0;
            index < tile_tbl->count;
            ++index) {

            if (map_id == tile_tbl->map_id_array[index]) {
                result = index * cfg.tile_capacity;
                break;
            }
        }

        return(result);
    }

    IFB_INTERNAL tile_map_id_u32
    tile_map_create(
        const cchar* name,
        const f32    tile_width,
        const f32    tile_height,
        const u32    count_rows,
        const u32    count_col) {

        assert(_tile_mngr  != NULL);
        assert(name        != NULL);
        assert(tile_width  != 0.0f);
        assert(tile_height != 0.0f);
        assert(count_rows  != 0);
        assert(count_col   != 0);

        // make sure we can create the map and
        // get the new index
        auto tbl = _tile_mngr->tbl_map;
        if (tbl->count == tbl->capacity) {
            return(INVALID_HASH_32);
        }
        const u32 index = tbl->count;

        // store the name and calculate hash
        auto& map_name = tbl->data.name[index];
        const u32 name_len = strnlen_s(name, 16);
        cchar* dst = &map_name.cstr[0];

        (void)strcpy_s(
            dst,
            16,
            name
        );

        const u32 map_id = tile_map_name_hash(map_name);
        assert(map_id != INVALID_HASH_32);

        // write the values to the table
        tbl->data.map_id      [index] = map_id;
        tbl->data.tile_width  [index] = tile_width;
        tbl->data.tile_height [index] = tile_height;
        tbl->data.count_rows  [index] = count_rows;
        tbl->data.count_cols  [index] = count_col;

        // TODO(SLD):
        // need to calculate the stride/offset for the tile table

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
        const u32 index = tile_map_index(tbl, map_id);
        assert(index != INVALID_INDEX);

        const u32 rows  = tbl->data.count_rows[index];
        const u32 cols  = tbl->data.count_cols[index];
        const u32 count = rows * cols;

        return(count);
    }

    IFB_INTERNAL void
    tile_map_set_color(
        const tile_map_id_u32 map_id,
        const u32*            row,
        const u32*            col,
        const color_rgba_u32* color,
        const u32             count) {

        assert(map_id     != INVALID_HASH_32);
        assert(row        != NULL);
        assert(col        != NULL);
        assert(color      != NULL);
        assert(count      != 0);
        assert(_tile_mngr != NULL); 
       
        auto tbl_map  = _tile_mngr->tbl_map;
        auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = tile_map_index    (tbl_map, map_id);
        const u32 offset     = tile_table_offset (tbl_tile, map_id);
        const u32 count_rows = tbl_map->data.count_rows[index];                
        const u32 count_cols = tbl_map->data.count_cols[index];

        for (
            u32 i = 0;
            i < count;
            ++i) {
       
            const u32 curr_row = row[i];
            const u32 curr_col = col[i];
            assert(curr_row < count_rows);
            assert(curr_col < count_cols);

            const u32 id = ((count_rows * curr_row) + curr_col) + offset;

            tbl_tile->data.color[id] = color[i];
        }
    }

    IFB_INTERNAL void
    tile_map_set_flags(
        const tile_map_id_u32 map_id,
        const u32*            row,
        const u32*            col,
        const tile_flags_u32* flags,
        const u32             count) {

        assert(map_id     != INVALID_HASH_32);
        assert(row        != NULL);
        assert(col        != NULL);
        assert(flags      != NULL);
        assert(count      != 0);
        assert(_tile_mngr != NULL); 
       
        auto tbl_map  = _tile_mngr->tbl_map;
        auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        const u32 index      = tile_map_index    (tbl_map, map_id);
        const u32 offset     = tile_table_offset (tbl_tile, map_id);
        const u32 count_rows = tbl_map->data.count_rows[index];                
        const u32 count_cols = tbl_map->data.count_cols[index];

        for (
            u32 i = 0;
            i < count;
            ++i) {
       
            const u32 curr_row = row[i];
            const u32 curr_col = col[i];
            assert(curr_row < count_rows);
            assert(curr_col < count_cols);

            const u32 id = ((count_rows * curr_row) + curr_col) + offset;

            tbl_tile->data.flags[id] = flags[i];
        }
    }

    IFB_INTERNAL bool
    tile_map_get_render_buffer(
        const tile_map_id_u32 map_id,
        tile_render_buffer*   render_buffer,
        arena*                a) {

        assert(map_id        != INVALID_HASH_32); 
        assert(render_buffer != NULL);
        assert(a             != NULL);

        const auto tbl_map  = _tile_mngr->tbl_map;
        const auto tbl_tile = _tile_mngr->tbl_tiles;
        assert(tbl_map);
        assert(tbl_tile);

        // get the map index
        const u32 map_index = tile_map_index(tbl_map, map_id);  
        assert(map_index != INVALID_HASH_32);

        // get row and column count
        const u32 count_rows  = tbl_map->data.count_rows[map_index];  
        const u32 count_cols  = tbl_map->data.count_cols[map_index];  
        assert(count_rows != 0);
        assert(count_cols != 0);

        // initialize the render buffer
        const u32 save = arena_save(a); 
        render_buffer->tile_count = count_rows * count_cols;
        render_buffer->data_size  = render_buffer->tile_count * sizeof(tile_render_context);  
        render_buffer->data.vptr  = arena_push(a, render_buffer->data_size);

        // return if we failed to initialize
        if (!render_buffer->data.vptr) {
            arena_revert(a, save);
            return(false);
        }
        arena_commit(a, save);

        // get the tile table offset
        const u32 tile_offset = tile_table_offset(tbl_tile, map_id);
        assert(tile_offset != INVALID_INDEX);
        const auto& cfg = config_instance();
        const u32 tile_index_max = tile_offset + cfg.tile_capacity;

        for (
            u32 index = 0;
            index <  render_buffer->tile_count;
            ++index) {

            const u32 tbl_index = tile_offset + index; 

            auto& ctx = render_buffer->data.ctx_array[index]; 
            ctx.id     = index;
            ctx.color  = tbl_tile->data.color[tbl_index].hex;
            ctx.corner = {0}; // TODO(SLD): need to figure out corner
        }

        return(true);
    }

};
