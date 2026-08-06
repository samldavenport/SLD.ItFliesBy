#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"

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
};

