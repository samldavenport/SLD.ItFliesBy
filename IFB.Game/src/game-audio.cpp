#pragma once

#include "ifb-game.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    IFB_GAME void
    game_audio_play_music(
        game_context* ctx,
        const cchar*  sound_name) {

        assert(ctx        != NULL);
        assert(sound_name != NULL);
        
        const auto audio = ctx->audio;
        assert(audio);

        if (audio->h_music != INVALID_HANDLE) {
            eng_audio_voice_stop(audio->h_music);
        }

        audio->h_music = eng_audio_sound_stream(
            sound_name,
            audio_group_e_music,
            GAME_MUSIC_VOLUME,
            true); 

        assert(audio->h_music != INVALID_HANDLE);
    }
    
    IFB_GAME void
    game_audio_play_sfx(
        game_context* ctx,
        const cchar* sound_name) {

        assert(ctx        != NULL);
        assert(sound_name != NULL);
        
        const auto audio = ctx->audio;
        assert(audio);
        
        //TODO(SLD): we need a simple _is_playing function that doesn't need this
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
