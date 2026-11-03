#ifndef ENG_AUDIO_HPP
#define ENG_AUDIO_HPP

#include "ifb.hpp"
#include "eng-memory.hpp"

namespace ifb {

    struct audio_mngr;

    IFB_INTERNAL audio_mngr*     audio_mngr_create      (void);
    IFB_INTERNAL void            audio_mngr_startup     (reservation* res);
    IFB_INTERNAL void            audio_mngr_shutdown    (void);
    IFB_INTERNAL void            audio_mngr_frame_start (void); // measures the device, and clears the buffer for this frame
    IFB_INTERNAL void            audio_mngr_frame_end   (void); // mixes the voices into the buffer, and sends it to the device
    IFB_INTERNAL bool            audio_mngr_get_buffer  (audio_buffer& buffer);
    IFB_INTERNAL void            audio_mngr_get_info    (audio_info& info);
    IFB_INTERNAL void            audio_mngr_reset_stats (void);

    // voices
    IFB_INTERNAL hnd_audio_voice audio_mngr_voice_play           (const wav* w, const f32 volume, const bool is_looping);
    IFB_INTERNAL void            audio_mngr_voice_stop           (const hnd_audio_voice h_voice);
    IFB_INTERNAL void            audio_mngr_voice_stop_now       (const hnd_audio_voice h_voice);
    IFB_INTERNAL void            audio_mngr_voice_stop_all_using (const wav* w);
    IFB_INTERNAL void            audio_mngr_voice_set_paused     (const hnd_audio_voice h_voice, const bool is_paused);
    IFB_INTERNAL void            audio_mngr_voice_set_volume     (const hnd_audio_voice h_voice, const f32 volume);
    IFB_INTERNAL void            audio_mngr_voice_set_looping    (const hnd_audio_voice h_voice, const bool is_looping);
    IFB_INTERNAL void            audio_mngr_voice_seek           (const hnd_audio_voice h_voice, const f64 position);
    IFB_INTERNAL bool            audio_mngr_voice_get_info       (const hnd_audio_voice h_voice, audio_voice_info& info);
};

#endif //ENG_AUDIO_HPP
