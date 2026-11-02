#ifndef ENG_AUDIO_HPP
#define ENG_AUDIO_HPP

#include "ifb.hpp"
#include "eng-memory.hpp"

namespace ifb {

    struct audio_mngr;

    IFB_INTERNAL audio_mngr* audio_mngr_create      (void);
    IFB_INTERNAL void        audio_mngr_startup     (reservation* res);
    IFB_INTERNAL void        audio_mngr_shutdown    (void);
    IFB_INTERNAL void        audio_mngr_frame_start (void); // measures the device, and clears the buffer for this frame
    IFB_INTERNAL void        audio_mngr_frame_end   (void); // sends the buffer to the device
    IFB_INTERNAL bool        audio_mngr_get_buffer  (audio_buffer& buffer);
    IFB_INTERNAL void        audio_mngr_get_info    (audio_info& info);
    IFB_INTERNAL void        audio_mngr_reset_stats (void);
};

#endif //ENG_AUDIO_HPP
