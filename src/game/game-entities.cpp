#pragma once

#include "ifb-game.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_entities_init(
        game_entities* entities,
        const hnd_map  h_map) {

        assert(entities);

        entities->static_entity = eng_entity_create("STATIC-ENTITY");
        assert(entities->static_entity != ENTITY_ID_INVALID);
    

        eng_entity_add_components(
            cmpnt_t
        );
    }

};
