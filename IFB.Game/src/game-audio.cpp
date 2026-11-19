#pragma once

#include "ifb-game.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    // a fade out in progress is called off, and the music goes back to its full volume
    // the volume can change while the voice is paused, it fades in to it when it's played
    IFB_INLINE void
    game_audio_music_cancel_fade(
        game_audio* audio) {

        if (audio->music_state != game_audio_music_state_e_fade_out) return;

        audio->music_state     = game_audio_music_state_e_none;
        audio->fade_out_volume = GAME_MUSIC_VOLUME;
        eng_audio_voice_set_volume(audio->h_music, GAME_MUSIC_VOLUME);
    }

    // the music is set without playing, the voice is kept until the music changes
    // the last song fades out while the new one waits, paused at the start
    // without the sound bank it fails, and the game runs without music
    IFB_GAME void
    game_audio_set_music(
        game_context* ctx,
        const cchar*  sound_name) {

        assert(ctx        != NULL);
        assert(sound_name != NULL);

        auto audio = ctx->audio;
        assert(audio);

        // an invalid or stale handle does nothing
        eng_audio_voice_destroy(audio->h_music);

        // the new song starts at full volume, even if the last one was fading out
        audio->music_state     = game_audio_music_state_e_none;
        audio->fade_out_volume = GAME_MUSIC_VOLUME;

        // streamed, so a long song isn't loaded all at once, its first chunks are read while it waits
        audio->h_music = eng_audio_voice_create(
            sound_name,
            audio_group_e_music,
            GAME_MUSIC_VOLUME,
            true,
            true);
    }

    // fades in from where it was paused, nothing happens without music set
    // a fade out is called off, so it comes back at full volume
    IFB_GAME void
    game_audio_play_music(
        game_context* ctx) {

        assert(ctx);

        auto audio = ctx->audio;
        assert(audio);

        game_audio_music_cancel_fade(audio);
        eng_audio_voice_play(audio->h_music);
    }

    // fades out and holds its place, it's still set
    // a fade out is called off, so it comes back at full volume when it's played
    IFB_GAME void
    game_audio_pause_music(
        game_context* ctx) {

        assert(ctx);

        auto audio = ctx->audio;
        assert(audio);

        game_audio_music_cancel_fade(audio);
        eng_audio_voice_pause(audio->h_music);
    }

    // turns the music down over GAME_MUSIC_FADE_OUT_S, from wherever its volume is, then stops it
    // stopped, it goes back to the start and pauses, so playing it again starts the song over at full volume
    // nothing happens if the music isn't playing, or is already fading out
    IFB_GAME void
    game_audio_fade_out_music(
        game_context* ctx) {

        assert(ctx);

        auto audio = ctx->audio;
        assert(audio);

        if (audio->music_state == game_audio_music_state_e_fade_out) return;

        audio_voice_info info;
        const bool is_set = eng_audio_voice_get_info(audio->h_music, info);
        if (!is_set || info.is_paused || info.is_stopping) return;

        audio->music_state     = game_audio_music_state_e_fade_out;
        audio->fade_out_volume = info.volume;
    }

    // moves the music's fade out along, once each update
    IFB_GAME void
    game_audio_update(
        game_context* ctx) {

        assert(ctx);

        auto audio = ctx->audio;
        assert(audio);

        if (audio->music_state != game_audio_music_state_e_fade_out) return;

        f32 dt_s = eng_system_get_delta_time_s();
        if (dt_s < 0.0f) dt_s = 0.0f;

        // the same rate from any volume, so a quieter song fades out sooner
        audio->fade_out_volume -= (GAME_MUSIC_VOLUME / GAME_MUSIC_FADE_OUT_S) * dt_s;
        if (audio->fade_out_volume > 0.0f) {
            // the engine fades each change over a few ms, so the steps don't click
            eng_audio_voice_set_volume(audio->h_music, audio->fade_out_volume);
            return;
        }

        // silent, it's stopped, and its volume is put back for the next time it's played
        // a stopping voice is silent whatever its volume, so this doesn't jump back up
        eng_audio_voice_stop       (audio->h_music);
        eng_audio_voice_set_volume (audio->h_music, GAME_MUSIC_VOLUME);
        audio->music_state     = game_audio_music_state_e_none;
        audio->fade_out_volume = GAME_MUSIC_VOLUME;
    }

    IFB_GAME void
    game_audio_play_sfx(
        game_context* ctx,
        const cchar* sound_name) {

        assert(ctx        != NULL);
        assert(sound_name != NULL);
        
        const auto audio = ctx->audio;
        assert(audio);
        
        audio_voice_info info;
        const bool is_boop_playing = eng_audio_voice_get_info(audio->h_boop, info);
        if (!is_boop_playing) {
            audio->h_boop = eng_audio_sound_play(
                sound_name,
                audio_group_e_sfx,
                GAME_BOOP_VOLUME,
                false
            );
        }
    }
    
};
