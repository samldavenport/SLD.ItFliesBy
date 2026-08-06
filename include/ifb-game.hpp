#ifndef IFB_GAME_HPP
#define IFB_GAME_HPP

#include "ifb-types.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    struct player_rig {
        entity_id connor_id;
        entity_id jig_id;
    }; 

    struct game_context {
        eng_arena_handle global_arena;
        player_rig * player_rig; 
    };

    IFB_INTERNAL game_context* game_context_create_and_init(void);

    IFB_INTERNAL void  player_rig_validate          (player_rig* rig);
    IFB_INTERNAL void  player_rig_init              (player_rig* rig);
    IFB_INTERNAL void  player_rig_update_and_render (player_rig* rig);
};

#endif //IFB_GAME_HPP
