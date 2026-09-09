#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld.hpp"
#include "map.hpp"

namespace ifb {

    IFB_ENGINE_API map_handle
    eng_map_create(
        const cchar* name) {

        assert(name != NULL);

        const map_handle hnd = map_create(name);
        return(hnd);
    } 
    
    IFB_ENGINE_API map_chunk_handle 
    eng_map_chunk_create(
        const map_handle         map_hnd,
        const u32                count_rows,
        const u32                count_cols,
        const s32                offset_row,
        const s32                offset_col,
        const map_tile_color_u32 base_color) {

        assert(map_hnd     != INVALID_HANDLE);
        assert(count_rows  != 0);
        assert(count_cols  != 0);
        
        return(0);
    } 

    IFB_ENGINE_API void
    eng_map_destroy(
        const map_handle map) {


    }

    IFB_ENGINE_API void
    eng_map_render(
        const map_handle map) {

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
