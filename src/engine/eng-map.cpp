#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld.hpp"
#include "map.hpp"

namespace ifb {

    IFB_ENGINE_API map_handle
    eng_map_create(
        const cchar* name,
        const u32    count_rows,
        const u32    count_cols) {

        assert(name       != NULL);
        assert(count_rows != 0);
        assert(count_cols != 0);

        const map_handle hnd = map_create(
            name,
            count_rows,
            count_cols
        );

        return(hnd);
    } 
    
    IFB_ENGINE_API u32 
    eng_map_chunk_create(
        const map_handle    map_hnd,
        const u32           count_rows,
        const u32           count_cols,
        const u32           origin_row,
        const u32           origin_col,
        const map_color_u32 base_color) {

        assert(map_hnd     != INVALID_HANDLE);
        assert(count_rows  != 0);
        assert(count_cols  != 0);
       
        const u32 chunk_index = map_chunk_create(
            map_hnd,
            count_rows,
            count_cols,
            origin_row,
            origin_col,
            base_color
        );

        return(chunk_index);
    } 

    IFB_ENGINE_API void
    eng_map_destroy(
        const map_handle map) {

        assert(map != INVALID_HANDLE);
    }

    IFB_ENGINE_API bool 
    eng_map_render(
        const map_handle map_hnd) {

        const bool result = renderer_tile_set_map(map_hnd);
        return(result);
    }

    IFB_ENGINE_API void
    eng_map_set_colors(
        const map_handle      map,
        const map_coords*     coords,
        const color_rgba_u32* color,
        const u32             count) {

        assert(map != INVALID_HANDLE);
        assert(coords   != NULL);
        assert(color    != NULL);
        assert(count    != 0);

    } 

    IFB_ENGINE_API bool 
    eng_map_get_entity_coords(
        const map_handle map,
        const entity_id  eid,
        map_coords&      coords) {

        return(false);
    }
};
