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

    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);

        eng_map_render(map->eng_hnd);
    } 
};
