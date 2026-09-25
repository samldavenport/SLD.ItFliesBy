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

        cstr_c16_init(&name, map_name);
        hnd = cstr_c16_hash(&name); 
        dims.count_chunks = 0;
        dims.count_rows   = count_rows;
        dims.count_cols   = count_cols;

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
                const u32 row = (chunk_tile_index / chunk.count_rows) + chunk.origin_row;
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
        const hnd_map           map_hnd,
        const cmpnt_map_coords& coords,
              cmpnt_position&   pos) {

        const auto& cfg = config_instance();
        
        // get the map dimensions;
        map_dimensions dims;
        if(!map_get_dimensions(map_hnd, dims)) {
            return(false);
        }

        map_calculate_position(
            cfg.map_tile_unit_size,
            dims,
            coords,
            pos
        );
    
        return(true);
    }

    IFB_INTERNAL bool
    map_get_coords_from_pos(
        const hnd_map           map_hnd, 
        const cmpnt_position&   pos,
              cmpnt_map_coords& coords) {

        const auto& cfg = config_instance();

        // get the map dimensions;
        map_dimensions dims;
        if (!map_get_dimensions(map_hnd, dims)) {
            return(false);
        }

        map_calculate_coordinates(
            map_hnd,
            cfg.map_tile_unit_size,
            dims,
            pos,
            coords
        );

        return(true);
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

            if (!e.archetype.has_any(cmpnt_type_e_map_coords)) {
                continue;
            } 

            cmpnt_lookup_map_coords(e.index_sparse, mc);
            if (mc.h_map != h_map) {
                continue;
            }    
        
            (void)entity_list_add(e_list, e.id);     
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
    map_get_dimensions(
        const hnd_map map_hnd,
        map_dimensions&  dims) {
      
        const u32 map_index = map_lookup_index(map_hnd);
        if (map_index == INVALID_INDEX) {
            return(false);
        }
        
        dims = map_get_dimensions(map_index);
        return(true);
    }
};
