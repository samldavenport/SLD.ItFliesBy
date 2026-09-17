#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {


        map->eng_hnd = eng_map_create("dev-map", 10, 10);
        assert(map->eng_hnd != INVALID_HANDLE);

        const u32 chunk_index = eng_map_chunk_create(
            map->eng_hnd,
            10, 10,
            0,  0, 
            map_color_e_purple_dark
        );
        assert(chunk_index != INVALID_INDEX);

        map->chunk_count = 1;
    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);
        assert(eng_map_render(map->eng_hnd));
    } 
};
