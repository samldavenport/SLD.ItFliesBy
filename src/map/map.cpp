#pragma once

#include "ifb-config.hpp"
#include "map-internal.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "sld.hpp"
#include "map.hpp"
#include <cassert>
#include <sld-strings.hpp>

namespace ifb {

    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL map_handle
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
        map_handle&      hnd    = tbl_map->hnd         [index];
        map_dimensions&  dims   = tbl_map->dims        [index]; 
        cstr_c16&        name   = tbl_map->name        [index];
        map_chunk_array& chunks = tbl_map->chunk_array [index];

        cstr_c16_init(&name, map_name);
        hnd = cstr_c16_hash(&name); 
        dims.count_chunks = 0;
        dims.count_rows   = count_rows;
        dims.count_cols   = count_cols;

        return(hnd);
    }

    IFB_INTERNAL bool 
    map_destroy(
        const map_handle map_hnd) {

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
    
    //--------------------------------------------------------------------
    // INTERNAL METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL u32
    map_lookup_index(
        const map_handle map_hnd) {

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
    
    IFB_INTERNAL const map_render_buffer*
    map_get_render_buffer(
        const map_handle   map_hnd,
        const arena_handle arena_hnd) {
        
        assert(map_hnd   != INVALID_HANDLE);
        assert(arena_hnd != INVALID_HANDLE);

        // get the map index
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(NULL);
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
            return(NULL);
        }

        // allocate buffer
        const u32 save        = arena_save(arena_hnd);
        const u32 size_struct = sizeof(map_render_buffer);
        const u32 size_data   = sizeof(map_tile) * tile_count; 
        auto*     buffer       = (map_render_buffer*)arena_push(arena_hnd, size_struct);
        auto*     tile_array   =          (map_tile*)arena_push(arena_hnd, size_data);
        if (buffer == NULL || tile_array == NULL) {
            arena_revert(arena_hnd, save);
            return(NULL);
        }
        arena_commit(arena_hnd, save);
      
        // initialize buffer
        buffer->map             = map_hnd;
        buffer->data_size       = size_data;
        buffer->data.tile_array = tile_array;

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
                const u32 row = (chunk_tile_index / chunk.count_rows) + chunk.origin_row;
                const u32 col = (chunk_tile_index % chunk.count_cols) + chunk.origin_col;

                // get the next tile
                map_tile& tile = buffer->data.tile_array[tile_index++]; 

                // initialize the tile
                tile.index = (row * dims.count_cols) + col;
                tile.color = (u8)chunk.base_color.val;
            }
        }
        return(buffer);
    } 
};
