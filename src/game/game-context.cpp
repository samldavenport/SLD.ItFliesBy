#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"

#include "game-player-rig.cpp"
#include "game-map.cpp"

namespace ifb {

    static game_context* _game_ctx;

    IFB_INTERNAL game_context*
    game_context_create_and_init(
        void) {

        hnd_arena arena_h = eng_arena_alloc();
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

        // load the sprite atlas
        ctx->test_sprites.h_atlas         = eng_sprite_atlas_load                ("TEST-ATLAS");
        ctx->test_sprites.i_sprite_connor = eng_sprite_atlas_lookup_sprite_index (ctx->test_sprites.h_atlas, "CONNOR");
        ctx->test_sprites.i_sprite_jig    = eng_sprite_atlas_lookup_sprite_index (ctx->test_sprites.h_atlas, "JIG");
        


        // the map goes first, the player rig spawns on it
        game_map_init        (ctx->map);
        rig->starting_map_hnd = map->eng_hnd;
        rig->current_map_hnd  = map->eng_hnd;
        game_player_rig_init (ctx->player_rig);

        _game_ctx = ctx;


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

        auto player_rig = ctx->player_rig;
        auto map        = ctx->map;

        game_player_rig_update_and_render (ctx->player_rig);
        game_player_rig_update_camera     (ctx->player_rig);
        game_map_update_and_render        (ctx->map);

        cmpnt_position pos;
        (void)eng_cmpnt_lookup_position(player_rig->connor_id, pos);
    }
    
    IFB_INTERNAL game_context*
    game_context_get_instance(
        void) {

        assert(_game_ctx);
        return(_game_ctx);
    }
};

