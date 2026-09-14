#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "map-internal.hpp"
#include "map.hpp"

namespace ifb {

    IFB_INTERNAL u32
    map_chunk_create(
        const map_handle     map_hnd,
        const u32            origin_row,
        const u32            origin_col,
        const u32            count_rows,
        const u32            count_cols,
        const color_rgba_u32 base_color) {

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
        if (dims.count_chunks == cfg.map_capacity) {
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

        //TODO(SLD): we need to make sure this chunk does not overlap
        // with any other existing chunks

        return(chunk_index);
    }

    IFB_INTERNAL bool 
    map_chunk_destroy(
        const map_handle map_hnd,
        const u32        chunk_index
    );
};
