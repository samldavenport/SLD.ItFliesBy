#pragma once

#include "ifb-game.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    IFB_GAME void
    game_audio_set_music(
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
};
