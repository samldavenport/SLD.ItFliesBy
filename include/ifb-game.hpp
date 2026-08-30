#ifndef IFB_GAME_HPP
#define IFB_GAME_HPP

#include "ifb-types.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    struct game_player_rig {
        entity_id connor_id;
        entity_id jig_id;
        entity_id jig_anchor_id;
    }; 

    struct game_map {
        eng_tile_map_handle eng_hnd;
    };

    struct game_context {
        eng_arena_handle    global_arena;
        game_player_rig*    player_rig; 
        game_map*           map;
    };

    IFB_INTERNAL game_context* game_context_create_and_init      (void);
    IFB_INTERNAL void          game_context_update_and_render    (game_context* ctx);

    IFB_INTERNAL void          game_player_rig_validate          (game_player_rig* player_rig);
    IFB_INTERNAL void          game_player_rig_init              (game_player_rig* player_rig);
    IFB_INTERNAL void          game_player_rig_update_and_render (game_player_rig* player_rig);

    IFB_INTERNAL void          game_map_init                     (game_map* map);
};

#endif //IFB_GAME_HPP
