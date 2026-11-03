#ifndef ENG_WAV_HPP
#define ENG_WAV_HPP

#include "ifb.hpp"

namespace ifb {

    // fourcc codes, as read from the file into a little-endian u32
    static constexpr u32 WAV_CSTR_RIFF      = 0x46464952; // "RIFF"
    static constexpr u32 WAV_CSTR_WAVE      = 0x45564157; // "WAVE"
    static constexpr u32 WAV_CSTR_FMT_      = 0x20746d66; // "fmt "
    static constexpr u32 WAV_CSTR_DATA      = 0x61746164; // "data"
    static constexpr u16 WAV_FORMAT_PCM     = 1;
    static constexpr u32 WAV_RIFF_HEADER    = 12;         // "RIFF", size, "WAVE"
    static constexpr u32 WAV_CHUNK_HEADER   = 8;          // id, size

    // the rate sounds are converted to when the asset database is built
    static constexpr u32 WAV_SOUND_SAMPLE_RATE = IFB_CONFIG_AUDIO_SAMPLE_RATE;

    IFB_INTERNAL bool wav_parse_in_place        (const byte* data, const u32 length, wav& w);
    IFB_INTERNAL wav* wav_parse_data            (const byte* data, const u32 length, const hnd_arena h_arena);
    IFB_INTERNAL f32  wav_read_sample           (const wav* w, const u32 frame, const u32 channel);
    IFB_INTERNAL u32  wav_convert_frame_count   (const wav& w, const u32 sample_rate);
    IFB_INTERNAL u32  wav_convert_channel_count (const wav& w);
    IFB_INTERNAL void wav_convert_to_s16        (const wav& w, const u32 sample_rate, s16* samples);

};

#endif //ENG_WAV_HPP
