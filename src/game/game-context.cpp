#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"

#include "game-player-rig.cpp"
#include "game-map.cpp"

namespace ifb {

    IFB_INTERNAL game_context*
    game_context_create_and_init(
        void) {

        eng_arena_handle arena_h = eng_arena_alloc();
        assert(arena_h != INVALID_INDEX);

        auto ctx =    (game_context*)eng_arena_push(arena_h, sizeof(game_context));
        auto rig = (game_player_rig*)eng_arena_push(arena_h, sizeof(game_player_rig));
        auto map =        (game_map*)eng_arena_push(arena_h, sizeof(game_map));

        assert(ctx != NULL);
        assert(rig != NULL);
        assert(map != NULL);

        ctx->global_arena = arena_h;
        ctx->player_rig   = rig;
        ctx->map          =  map; 

        game_player_rig_init (ctx->player_rig);
        game_map_init        (ctx->map);

        return(ctx);      
    }
    
    IFB_INTERNAL void
    game_context_validate(
        game_context* ctx) {

        assert(ctx);
        assert(ctx->player_rig   != NULL);
        assert(ctx->global_arena != INVALID_INDEX);
    }

    IFB_INTERNAL void
    game_context_update_and_render(
        game_context* ctx) {

        game_context_validate(ctx);

        game_player_rig_update_and_render (ctx->player_rig);
        game_map_update_and_render        (ctx->map);
    }
};

