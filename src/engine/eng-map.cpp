#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld.hpp"
#include "map.cpp"
#include "map.hpp"

namespace ifb {

    IFB_ENGINE_API eng_map_handle
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

        const map_id_u32 id = map_create(
            name,
            count_rows,
            count_cols,
            offset_row,
            offset_col,
            base_color
        );

        const eng_map_handle hnd = {id.val};
        return(hnd);
    } 

    IFB_ENGINE_API void
    eng_map_destroy(
        const eng_map_handle map) {

        const map_id_u32 id = { map.val };

        map_destroy(id);
    }

    IFB_ENGINE_API void
    eng_map_render(
        const eng_map_handle map) {

        const map_id_u32 id = { map.val };

        renderer_tile_set_map(id);
    }

    IFB_ENGINE_API void
    eng_map_set_colors(
        const eng_map_handle  map,
        const tile_coords*    coords,
        const color_rgba_u32* color,
        const u32             count) {

        assert(map != INVALID_HANDLE);
        assert(coords   != NULL);
        assert(color    != NULL);
        assert(count    != 0);

        map_id_u32 map_id = { map.val };
        map_set_color(
            map_id,
            coords,
            color,
            count
        );
    } 

    IFB_ENGINE_API void
    eng_map_set_flags(
        const eng_map_handle map,
        const tile_coords*        coords,
        const tile_flags_u32*     flags,
        const u32                 count) {

        assert(map != INVALID_HANDLE);
        assert(coords   != NULL);
        assert(flags    != NULL);
        assert(count    != 0);

        map_id_u32 map_id = { map.val };
        map_set_flags(
            map_id,
            coords,
            flags,
            count
        );
    } 
};
