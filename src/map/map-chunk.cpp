#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "map-internal.hpp"
#include "map.hpp"
#include "map.cpp"

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

        // get the chunk
        const u32  chunk_index = dims.count_chunks;
        map_chunk& chunk       = map_get_chunk(map_index, chunk_index);
    
        // set the chunk properties
        chunk.origin_row = origin_row;
        chunk.origin_col = origin_col;
        chunk.count_rows = count_rows;
        chunk.count_cols = count_cols;
        chunk.base_color = base_color;

        // update the chunk count
        ++dims.count_chunks;
        
        // TODO(SLD): we need to make sure this chunk does not overlap
        // with any other existing chunks

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

    IFB_INTERNAL bool
    map_chunk_destroy(
        const hnd_map map_hnd,
        const u32        chunk_index) {

        //TODO
        return(false);
    }
};
