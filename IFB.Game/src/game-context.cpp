#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-player-rig.cpp"
#include "game-map.cpp"

namespace ifb {

    IFB_INTERNAL void
    game_phys_cllsn_handler(
        const event_payload& payload) {

        assert(_game_ctx);
        assert(_game_ctx->entities);
        assert(_game_ctx->player_rig);

        const auto  entities   = _game_ctx->entities;
        const auto  player_rig = _game_ctx->player_rig;
        const auto& cllsn_data = payload.data.physics_collision; 

        const bool has_connor = (cllsn_data.a == player_rig->connor_id || cllsn_data.b == player_rig->connor_id); 
        const bool has_box    = (
                cllsn_data.a == entities->static_entity_0 ||
                cllsn_data.b == entities->static_entity_0 ||
                cllsn_data.a == entities->static_entity_1 ||
                cllsn_data.b == entities->static_entity_1 ||
                cllsn_data.a == entities->static_entity_2 ||
                cllsn_data.b == entities->static_entity_2 ||
                cllsn_data.a == entities->static_entity_3 ||
                cllsn_data.b == entities->static_entity_3
        ); 

        if (!has_connor || !has_box) return;

        // only a new bump, not every step connor stays against the box
        auto       audio          = _game_ctx->audio;
        const bool is_new_contact = (audio->time_s - audio->box_contact_time_s) >= GAME_BOOP_CONTACT_GAP_S;
        audio->box_contact_time_s = audio->time_s;
        if (!is_new_contact) return;

        // and never on top of the last boop, the handle goes stale when it finishes
        audio_voice_info info;
        const bool is_boop_playing = eng_audio_voice_get_info(audio->h_boop, info);
        if (is_boop_playing) return;

        audio->h_boop = eng_audio_sound_play("BOOP", GAME_BOOP_VOLUME, false);
    }

    IFB_INTERNAL game_context*
    game_context_create_and_init(
        void) {

        hnd_arena arena_h = eng_arena_alloc();
        assert(arena_h != INVALID_INDEX);

        auto ctx      =    (game_context*)eng_arena_push(arena_h, sizeof(game_context));
        auto rig      = (game_player_rig*)eng_arena_push(arena_h, sizeof(game_player_rig));
        auto map      =        (game_map*)eng_arena_push(arena_h, sizeof(game_map));
        auto entities =   (game_entities*)eng_arena_push(arena_h, sizeof(game_entities));
        auto sprites  =    (game_sprites*)eng_arena_push(arena_h, sizeof(game_sprites));
        auto audio    =      (game_audio*)eng_arena_push(arena_h, sizeof(game_audio));

        assert(ctx      != NULL);
        assert(rig      != NULL);
        assert(map      != NULL);
        assert(entities != NULL);
        assert(sprites  != NULL);
        assert(audio    != NULL);

        ctx->global_arena = arena_h;
        ctx->player_rig   = rig;
        ctx->map          = map;
        ctx->entities     = entities;
        ctx->sprites      = sprites;
        ctx->audio        = audio;

        // nothing is playing, and the first contact counts as a bump
        audio->h_boop             = {INVALID_HANDLE};
        audio->time_s             = 0.0;
        audio->box_contact_time_s = -GAME_BOOP_CONTACT_GAP_S;

        // load the sprite atlas
        sprites->h_atlas         = eng_sprite_atlas_load                ("CORE-SPRITES");
        sprites->i_sprite_connor = eng_sprite_atlas_lookup_sprite_index (sprites->h_atlas, "CONNOR");
        sprites->i_sprite_jig    = eng_sprite_atlas_lookup_sprite_index (sprites->h_atlas, "JIG");

        // set the sprite atlas to active
        const bool did_set_texture = eng_sprite_atlas_set_active(sprites->h_atlas);
        assert(did_set_texture);
        (void)did_set_texture;


        // the map goes first, the player rig spawns on it
        game_map_init        (ctx->map);

        rig->starting_map_hnd = map->eng_hnd;
        rig->current_map_hnd  = map->eng_hnd;
        game_player_rig_init (ctx->player_rig);

        entities->static_entity_0 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-0","WOOD",map->eng_hnd,0,3,3);
        entities->static_entity_1 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-1","WOOD",map->eng_hnd,0,3,7);
        entities->static_entity_2 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-2","WOOD",map->eng_hnd,0,7,3);
        entities->static_entity_3 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-3","WOOD",map->eng_hnd,0,7,7);

        assert(entities->static_entity_0 != ENTITY_ID_INVALID);
        assert(entities->static_entity_1 != ENTITY_ID_INVALID);
        assert(entities->static_entity_2 != ENTITY_ID_INVALID);
        assert(entities->static_entity_3 != ENTITY_ID_INVALID);


        const bool did_subscribe = eng_event_listener_subscribe(event_type_e_physics_collision, game_phys_cllsn_handler);
        assert(did_subscribe);

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
        auto entities   = ctx->entities;

        // the same step the physics uses, so the contact gap is measured in the time the collisions happen in
        ctx->audio->time_s += eng_system_get_delta_time_s();

        game_player_rig_update_and_render (ctx->player_rig);
        game_player_rig_update_camera     (ctx->player_rig);
        game_map_update_and_render        (ctx->map);

        eng_entity_render(entities->static_entity_0);
        eng_entity_render(entities->static_entity_1);
        eng_entity_render(entities->static_entity_2);
        eng_entity_render(entities->static_entity_3);
 
        static const cchar test_text[] = "HELLO FROM JOB WORKER";

        job test_job;
        cstr_nvar_copy(
            test_text,
            sizeof(test_text),
            (cchar*)test_job.ctx.data,
            sizeof(test_job.ctx)
        );

    }


};
