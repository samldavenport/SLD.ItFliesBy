#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {

        assert(map);

        map->eng_hnd = eng_tile_map_create("DEV MAP", 0.2f, 0.2f, 4, 4);
        assert(map->eng_hnd != INVALID_HANDLE);
    }
    
};
