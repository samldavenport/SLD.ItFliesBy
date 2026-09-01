#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {

        assert(map);

        const color_rgba_u32 base_color(0xB16286FF); 

        map->eng_hnd = eng_map_create("DEV MAP", 0.2f, 0.2f, 4, 4, base_color);
        assert(map->eng_hnd != INVALID_HANDLE);
    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);

        eng_map_render(map->eng_hnd);
    } 
};
