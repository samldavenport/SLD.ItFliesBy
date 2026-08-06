#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"

#include "game-player-rig.cpp"

namespace ifb {

    IFB_INTERNAL game_context*
    game_context_create_and_init(
        void) {

        eng_arena_handle arena_h = eng_arena_alloc();
        assert(arena_h != INVALID_INDEX);

        auto ctx = (game_context*)eng_arena_push(arena_h, sizeof(game_context));
        auto rig =   (player_rig*)eng_arena_push(arena_h, sizeof(player_rig));

        assert(ctx != NULL);
        assert(rig != NULL);

        ctx->global_arena = arena_h;
        ctx->player_rig   = rig;

        player_rig_init(ctx->player_rig);

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

        player_rig_update_and_render(ctx->player_rig);
    }
};

