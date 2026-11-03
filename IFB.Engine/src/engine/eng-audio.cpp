#pragma once

#include "ifb-engine.hpp"
#include "eng-audio.hpp"

namespace ifb {

    IFB_ENGINE_API bool
    eng_audio_get_buffer(
        audio_buffer& buffer) {

        return(audio_mngr_get_buffer(buffer));
    }

    IFB_ENGINE_API void
    eng_audio_get_info(
        audio_info& info) {

        audio_mngr_get_info(info);
    }

    IFB_ENGINE_API void
    eng_audio_reset_stats(
        void) {

        audio_mngr_reset_stats();
    }

    IFB_ENGINE_API hnd_audio_voice
    eng_audio_voice_play(
        const wav*          w,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        return(audio_mngr_voice_play(w, group, volume, is_looping));
    }

    IFB_ENGINE_API hnd_audio_voice
    eng_audio_sound_play(
        const cchar*        name,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        assert(name);
        return(audio_mngr_voice_play_sound(name, group, volume, is_looping));
    }

    IFB_ENGINE_API void
    eng_audio_group_set_volume(
        const audio_group_e group,
        const f32           volume) {

        audio_mngr_group_set_volume(group, volume);
    }

    IFB_ENGINE_API void
    eng_audio_set_master_volume(
        const f32 volume) {

        audio_mngr_set_master_volume(volume);
    }

    IFB_ENGINE_API f32
    eng_audio_get_master_volume(
        void) {

        return(audio_mngr_get_master_volume());
    }

    IFB_ENGINE_API f32
    eng_audio_group_get_volume(
        const audio_group_e group) {

        return(audio_mngr_group_get_volume(group));
    }

    IFB_ENGINE_API bool
    eng_audio_sound_preload(
        const cchar* name) {

        assert(name);
        return(audio_mngr_sound_preload(name));
    }

    IFB_ENGINE_API void
    eng_audio_sound_unload(
        const cchar* name) {

        assert(name);
        audio_mngr_sound_unload(name);
    }

    IFB_ENGINE_API void
    eng_audio_voice_stop(
        const hnd_audio_voice h_voice) {

        audio_mngr_voice_stop(h_voice);
    }

    IFB_ENGINE_API void
    eng_audio_voice_stop_now(
        const hnd_audio_voice h_voice) {

        audio_mngr_voice_stop_now(h_voice);
    }

    IFB_ENGINE_API void
    eng_audio_voice_stop_all_using(
        const wav* w) {

        audio_mngr_voice_stop_all_using(w);
    }

    IFB_ENGINE_API void
    eng_audio_voice_set_paused(
        const hnd_audio_voice h_voice,
        const bool            is_paused) {

        audio_mngr_voice_set_paused(h_voice, is_paused);
    }

    IFB_ENGINE_API void
    eng_audio_voice_set_volume(
        const hnd_audio_voice h_voice,
        const f32             volume) {

        audio_mngr_voice_set_volume(h_voice, volume);
    }

    IFB_ENGINE_API void
    eng_audio_voice_set_looping(
        const hnd_audio_voice h_voice,
        const bool            is_looping) {

        audio_mngr_voice_set_looping(h_voice, is_looping);
    }

    IFB_ENGINE_API void
    eng_audio_voice_seek(
        const hnd_audio_voice h_voice,
        const f64             position) {

        audio_mngr_voice_seek(h_voice, position);
    }

    IFB_ENGINE_API bool
    eng_audio_voice_get_info(
        const hnd_audio_voice h_voice,
        audio_voice_info&     info) {

        return(audio_mngr_voice_get_info(h_voice, info));
    }
};
