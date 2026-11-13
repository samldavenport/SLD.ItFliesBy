#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-scene-gameplay.cpp"
#include "game-dialog.cpp"

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

        // only a new bump, not every step connor stays against the box, or when he walks away
        if (cllsn_data.phase != phys_cllsn_phase_e_begin) return;

        auto audio = _game_ctx->audio;

        // the bump opens a dialog, unless one is already up
        // connor is still touching the box when it closes, but that isn't a new bump, so it doesn't open again
        // the events are sent after the game callback, so the dialog shows from the next frame
        if (_game_ctx->fnt != NULL && !game_dialog_is_open(_game_ctx->dialog)) {
            static constexpr cchar collision_text[] = "A collision happened...";
            game_dialog_open(_game_ctx->dialog, _game_ctx->fnt, collision_text, sizeof(collision_text) - 1);
        }

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
        auto dialog   =     (game_dialog*)eng_arena_push(arena_h, sizeof(game_dialog));

        assert(ctx      != NULL);
        assert(sprites  != NULL);
        assert(audio    != NULL);
        assert(dialog   != NULL);

        // the gameplay scene fills in the rest
        ctx->global_arena     = arena_h;
        ctx->h_scene_gameplay = INVALID_HANDLE;
        ctx->player_rig       = NULL;
        ctx->map              = NULL;
        ctx->entities         = NULL;
        ctx->npc_mngr         = NULL;
        ctx->sprites          = sprites;
        ctx->audio            = audio;
        ctx->fnt              = NULL;
        ctx->dialog           = dialog;
        ctx->talk_was_down    = false;
        game_dialog_init(dialog);

        // nothing is playing yet
        audio->h_boop = {INVALID_HANDLE};

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

        // the ui font, copied onto the global arena so it lives as long as the game
        // without the asset database it fails to load, and the game runs without text
        font* fnt = eng_font_atlas_load_to_arena("PRESS-START-16", arena_h);
        if (fnt != NULL && eng_font_upload(fnt) && eng_font_set_active(fnt)) {
            ctx->fnt = fnt;
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

        // connor stands still while he's being talked to
        ctx->player_rig->is_input_locked = game_dialog_is_open(ctx->dialog);

        game_scene_gameplay_update_and_render(ctx);

        // the dialog needs the font, without it there's nothing to show
        if (ctx->fnt == NULL) return;


        const bool is_talk_down   = eng_input_is_key_down(input_keycode_e_e);
        const bool is_talk_press  = is_talk_down && !ctx->talk_was_down;
        ctx->talk_was_down        = is_talk_down;

        auto dialog = ctx->dialog;
        if (is_talk_press) {
            if (game_dialog_is_open(dialog)) {
                game_dialog_advance(dialog);
            }
        }

        game_dialog_update (dialog, eng_system_get_delta_time_s());
        game_dialog_render (dialog, ctx->fnt);
    }


};
