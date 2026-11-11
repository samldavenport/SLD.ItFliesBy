#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-scene-gameplay.cpp"

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

        audio->h_boop = eng_audio_sound_play("BOOP", audio_group_e_sfx, GAME_BOOP_VOLUME, false);
    }

    IFB_INTERNAL game_context*
    game_context_create_and_init(
        void) {

        hnd_arena arena_h = eng_arena_alloc();
        assert(arena_h != INVALID_INDEX);

        auto ctx      =    (game_context*)eng_arena_push(arena_h, sizeof(game_context));
        auto sprites  =    (game_sprites*)eng_arena_push(arena_h, sizeof(game_sprites));
        auto audio    =      (game_audio*)eng_arena_push(arena_h, sizeof(game_audio));

        assert(ctx      != NULL);
        assert(sprites  != NULL);
        assert(audio    != NULL);

        // the gameplay scene fills in the rest
        ctx->global_arena     = arena_h;
        ctx->h_scene_gameplay = INVALID_HANDLE;
        ctx->player_rig       = NULL;
        ctx->map              = NULL;
        ctx->entities         = NULL;
        ctx->sprites          = sprites;
        ctx->audio            = audio;

        // nothing is playing, and the first contact counts as a bump
        audio->h_boop             = {INVALID_HANDLE};
        audio->time_s             = 0.0;
        audio->box_contact_time_s = -GAME_BOOP_CONTACT_GAP_S;

        // loaded now so the first bump doesn't wait on the disk, it stays loaded for the whole game
        // without the sound bank it fails, and the boop just doesn't play
        (void)eng_audio_sound_preload("BOOP");

        // the music is streamed from the sound bank instead of loaded, and loops for the whole game
        // it's in the music group, so it can be turned down without the sound effects
        audio->h_music = eng_audio_sound_stream("PIANO", audio_group_e_music, GAME_MUSIC_VOLUME, true);

        // load the sprite atlas
        sprites->h_atlas         = eng_sprite_atlas_load                ("CORE-SPRITES");
        sprites->i_sprite_connor = eng_sprite_atlas_lookup_sprite_index (sprites->h_atlas, "CONNOR");
        sprites->i_sprite_jig    = eng_sprite_atlas_lookup_sprite_index (sprites->h_atlas, "JIG");

        // set the sprite atlas to active
        // without the asset database the atlas fails to load, and the game runs without sprites
        // so the database can still be rebuilt from the tools
        if (sprites->h_atlas != INVALID_HANDLE) {
            const bool did_set_texture = eng_sprite_atlas_set_active(sprites->h_atlas);
            assert(did_set_texture);
            (void)did_set_texture;
        }


        // the level
        game_scene_gameplay_init(ctx);

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

        auto audio = ctx->audio;

        eng_audio_voice_stop(audio->h_music);

        audio->time_s += eng_system_get_delta_time_s();

        game_scene_gameplay_update_and_render(ctx);
    }


};
