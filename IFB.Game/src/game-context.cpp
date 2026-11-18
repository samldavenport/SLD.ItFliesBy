#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-scene.cpp"
#include "game-dialog.cpp"

namespace ifb {

    IFB_GAME game_context*
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

        // the ui font, copied onto the global arena so it lives as long as the game
        // the calls stay outside the assert so they still run when asserts are compiled out
        font*      fnt           = eng_font_atlas_load_to_arena("PRESS-START-16", arena_h);
        const bool is_font_ready = (
            fnt != NULL             &&
            eng_font_upload(fnt)    &&
            eng_font_set_active(fnt)
        );
        assert(is_font_ready);
        (void)is_font_ready;

        // the active scene fills in the player rig and map
        // the npc manager outlives the scenes, so it can keep npcs from one scene to the next
        ctx->global_arena      = arena_h;
        ctx->player_rig        = NULL;
        ctx->map               = NULL;
        ctx->phys_test         = NULL;
        ctx->npc_mngr          = game_npc_mngr_create(arena_h);
        ctx->sprites           = sprites;
        ctx->audio             = audio;
        ctx->fnt               = fnt;
        ctx->dialog            = game_dialog_create(arena_h, fnt);
        ctx->camera_focus      = {0}; // the engine's default camera is framed around the world origin
        ctx->talk_was_down     = false;
        ctx->is_action_pressed = false;

        // nothing is playing yet
        audio->h_boop = {INVALID_HANDLE};

        // loaded now so the first bump doesn't wait on the disk, it stays loaded for the whole game
        // without the sound bank it fails, and the boop just doesn't play
        (void)eng_audio_sound_preload("BOOP");

        // the music is streamed from the sound bank instead of loaded, and loops for the whole game
        // it's in the music group, so it can be turned down without the sound effects
        audio->h_music = eng_audio_sound_stream("AMBIENT-LOOP", audio_group_e_music, GAME_MUSIC_VOLUME, true);

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


        // set before the first scene enters, the collision procs read it
        _game_ctx = ctx;

        game_scene_init(ctx, game_scene_id_e_dev);

        return(ctx);
    }
    
    IFB_GAME void
    game_context_validate(
        game_context* ctx) {

        assert(ctx);
        assert(ctx->global_arena != INVALID_INDEX);
        assert(ctx->npc_mngr     != NULL);
    }

    
    IFB_GAME void
    game_context_update_and_render(
        game_context* ctx) {

        IFB_PROFILE_FUNCTION();

        game_context_validate(ctx);

        auto audio = ctx->audio;

        eng_audio_voice_stop(audio->h_music);

        game_scene_update_and_render(ctx);
        game_dialog_update(ctx);

        // the dialog leaves e for the collision procs, nothing gets it while a scene fades
        if (game_scene_is_switching()) {
            ctx->is_action_pressed = false;
        }

        // last, so it's over the scene and the dialog
        game_scene_render_fade();
    }


};
