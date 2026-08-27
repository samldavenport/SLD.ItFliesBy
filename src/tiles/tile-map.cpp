#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "tile.hpp"

namespace ifb {
   
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
    tile_table_get_id (
        const tile_table* tile_tbl,
        const u32         tile_index) {

        assert(tile_tbl);
        assert(tile_tbl->data.row);
        assert(tile_tbl->data.col);
      
        tile_id_composite composite;
        composite.row = tile_tbl->data.row[tile_index];
        composite.col = tile_tbl->data.col[tile_index];
     
        return(composite.id);
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
        (void)strncpy_s(map_name.cstr, TILE_MAP_NAME_LENGTH, name, TILE_MAP_NAME_LENGTH);
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

    IFB_INTERNAL void tile_map_destroy           (const tile_map_id_u32 map_id);

    IFB_INTERNAL u32
    tile_map_tile_count(
        const tile_map_id_u32 map_id) {


        assert(map_id != INVALID_HASH_32);

        // get the index for the map
        

    }

    IFB_INTERNAL void tile_map_set_color         (const tile_map_id_u32 map_id, const tile_id_u32* id, const color_rgba_u32* color, const u32 count);
    IFB_INTERNAL void tile_map_set_flags         (const tile_map_id_u32 map_id, const tile_id_u32* id, const tile_flags_u32* flags, const u32 count);

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
            u32 tile_index = 0;
            tile_index <  render_buffer->tile_count;
            ++tile_index) {

            const u32 tbl_index = tile_index + tile_offset;
           
            auto& ctx = render_buffer->data.ctx_array[tile_index]; 
            ctx.id     = tile_table_get_id(tbl_tile, tbl_index);
            ctx.color  = tbl_tile->data.color[tbl_index].hex;
            ctx.corner = {0}; // TODO(SLD): need to figure out corner
        }

        return(true);
    }

};
