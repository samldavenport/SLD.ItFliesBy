#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {


        map->eng_hnd = eng_map_create("dev-map", 25, 25);
        assert(map->eng_hnd != INVALID_HANDLE);

        const u32 chunk_index_0 = eng_map_chunk_create(map->eng_hnd, 10,  10,  0, 0, map_color_e_purple_dark);
        const u32 chunk_index_1 = eng_map_chunk_create(map->eng_hnd,  5,   5, 5, 10, map_color_e_red_light);
        const u32 chunk_index_2 = eng_map_chunk_create(map->eng_hnd,  10, 10, 10, 5, map_color_e_red_dark);
        assert(chunk_index_0 != INVALID_INDEX);
        assert(chunk_index_1 != INVALID_INDEX);

        map->chunk_count = 2;
    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);
        assert(eng_map_render(map->eng_hnd));
    } 
};
