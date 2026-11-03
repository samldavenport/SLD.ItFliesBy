#pragma once

#include "ifb.hpp"
#include "eng-wav.hpp"
#include "eng-asset.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // where a voice gets its samples from, the mixer only reads frames through it
    // a memory source reads a wav that's already in memory, a stream source will read chunks of a sound from the bank
    enum audio_source_type_e {
        audio_source_type_e_none   = 0,
        audio_source_type_e_memory = 1
    };

    // the source gives the mixer at most 2 channels, a mono source stays mono and extra channels in the wav are dropped
    // a sound asset stays pinned for as long as the source has it, so its samples can't move
    struct audio_source {
        u32        type;
        u32        frame_count;
        u32        sample_rate;
        u32        channel_count; // 1 or 2, what the source gives the mixer
        const wav* w;             // memory
        hnd_asset  h_sound;       // INVALID_HANDLE if the wav isn't a sound asset
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the source takes over the pin on h_sound, if there is one
    inline audio_source
    audio_source_from_memory(
        const wav*      w,
        const hnd_asset h_sound) {

        assert(w && w->format.block_align != 0);

        audio_source src;
        src.type          = audio_source_type_e_memory;
        src.frame_count   = (w->data.size / w->format.block_align);
        src.sample_rate   = w->format.sample_rate;
        src.channel_count = (w->format.num_channels == 1) ? 1 : 2;
        src.w             = w;
        src.h_sound       = h_sound;
        return(src);
    }

    // reads count frames starting at frame, interleaved with channel_count samples each, from -1 to 1
    // the frames have to be inside the source, the mixer handles looping and the end
    // returns the frames read, which a stream will be able to come up short on
    inline u32
    audio_source_read(
        const audio_source& src,
        const u32           frame,
        const u32           count,
        f32*                out) {

        assert(out);
        assert((u64)frame + count <= src.frame_count);

        switch (src.type) {

            case (audio_source_type_e_memory): {

                const wav* w        = src.w;
                const u32  channels = src.channel_count;

                // every sound asset is 16 bit, so it's read directly
                // the samples aren't aligned in every wav, so they're copied out instead of cast
                if (w->format.bits_per_sample == 16) {

                    const u32   block_align = w->format.block_align;
                    const byte* bytes       = w->data.bytes + ((u64)frame * block_align);
                    for (
                        u32 i_frame = 0;
                            i_frame < count;
                          ++i_frame) {

                        for (
                            u32 i_channel = 0;
                                i_channel < channels;
                              ++i_channel) {

                            s16 value = 0;
                            memcpy(&value, bytes + (i_frame * block_align) + (i_channel * sizeof(s16)), sizeof(s16));
                            out[(i_frame * channels) + i_channel] = (f32)value / 32768.0f;
                        }
                    }
                }
                else {

                    for (
                        u32 i_frame = 0;
                            i_frame < count;
                          ++i_frame) {

                        for (
                            u32 i_channel = 0;
                                i_channel < channels;
                              ++i_channel) {

                            out[(i_frame * channels) + i_channel] = wav_read_sample(w, frame + i_frame, i_channel);
                        }
                    }
                }
                return(count);
            }
        }
        return(0);
    }

    // lets go of the sound asset, if the source has one pinned
    inline void
    audio_source_release(
        audio_source& src) {

        if (src.h_sound != INVALID_HANDLE) {
            asset_mngr_unpin(src.h_sound);
        }
        src = {};
        src.h_sound = {INVALID_HANDLE};
    }
};
