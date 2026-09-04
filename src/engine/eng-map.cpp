#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld.hpp"
#include "map.cpp"
#include "map.hpp"

namespace ifb {

    IFB_ENGINE_API map_handle
    eng_map_create(
        const cchar*         name,
        const u32            count_rows,
        const u32            count_cols,
        const s32            offset_row,
        const s32            offset_col,
        const color_rgba_u32 base_color) {

        assert(name        != NULL);
        assert(count_rows  != 0);
        assert(count_cols  != 0);

        const map_handle hnd = map_create(
            name,
            count_rows,
            count_cols,
            offset_row,
            offset_col,
            base_color
        );

        return(hnd);
    } 

    IFB_ENGINE_API void
    eng_map_destroy(
        const map_handle map) {


        map_destroy(map);
    }

    IFB_ENGINE_API void
    eng_map_render(
        const map_handle map) {

        renderer_tile_set_map(map);
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

        map_set_color(
            map,
            coords,
            color,
            count
        );
    } 

    IFB_ENGINE_API bool 
    eng_map_get_entity_coords(
        const map_handle map,
        const entity_id  eid,
        map_coords&      coords) {

        const bool did_find = map_get_entity_tile_coordinates(map, eid, coords);
        return(did_find);
    }
};
