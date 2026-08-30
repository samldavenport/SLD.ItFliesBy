#pragma once

#include "ifb-engine.hpp"
#include "tile-map.cpp"
#include "tile.hpp"

namespace ifb {

    IFB_ENGINE_API eng_tile_map_handle
    eng_tile_map_create(
        const cchar* name,
        const f32    tile_width,
        const f32    tile_height,
        const u32    count_rows,
        const u32    count_cols) {

        assert(name        != NULL);
        assert(tile_width  >  0.0f);
        assert(tile_height >  0.0f);
        assert(count_rows  != 0);
        assert(count_cols  != 0);

        const tile_map_id_u32 id = tile_map_create(
            name,
            tile_width,
            tile_height,
            count_rows,
            count_cols
        );

        const eng_tile_map_handle hnd = {id.val};
        return(hnd);
    } 

    IFB_ENGINE_API void
    eng_tile_map_destroy(
        const eng_tile_map_handle tile_map) {

        const tile_map_id_u32 id = { tile_map.val };

        tile_map_destroy(id);
    }

    IFB_ENGINE_API void
    eng_tile_map_render(
        const eng_tile_map_handle tile_map) {

        //TODO(SLD)
    }

    IFB_ENGINE_API void
    eng_tile_map_set_colors(
        const eng_tile_map_handle tile_map,
        const u32*                row,
        const u32*                col,
        const color_rgba_u32*     color,
        const u32                 count) {

    } 

    IFB_ENGINE_API void
    eng_tile_map_set_flags(
        const eng_tile_map_handle tile_map,
        const u32*                row,
        const u32*                col,
        const u32*                flags,
        const u32                 count) {

    } 
};
