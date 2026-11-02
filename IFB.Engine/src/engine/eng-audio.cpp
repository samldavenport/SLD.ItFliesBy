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
};
