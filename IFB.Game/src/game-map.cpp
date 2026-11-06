#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {

        // DEV-MAP is assets/maps/dev-map.json, a text asset in the asset database
        // it uses CORE-SPRITES, so the atlas has to be loaded first
        map->eng_hnd = eng_map_load("DEV-MAP");
        assert(map->eng_hnd != INVALID_HANDLE);
    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);
        const bool did_render = eng_map_render(map->eng_hnd);
        assert(did_render);
    } 
};
