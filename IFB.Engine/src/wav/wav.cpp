#pragma once

#include "eng-wav.hpp"
#include "eng-memory.hpp"

namespace ifb {

    IFB_INTERNAL wav*
    wav_parse_data(
        const byte*     data,
        const u32       length,
        const hnd_arena h_arena) {

        assert(h_arena != INVALID_HANDLE);

        if (!data || length < WAV_RIFF_HEADER) return(NULL);

        // validate the riff header
        u32 riff_id = 0;
        u32 wave_id = 0;
        memcpy(&riff_id, data,     sizeof(u32));
        memcpy(&wave_id, data + 8, sizeof(u32));
        if (riff_id != WAV_CSTR_RIFF || wave_id != WAV_CSTR_WAVE) return(NULL);

        wav_format  format    = {};
        bool        has_fmt   = false;
        const byte* samples   = NULL;
        u32         data_size = 0;

        // chunks can be in any order, and we skip the ones we don't use
        u32 pos = WAV_RIFF_HEADER;
        while (length - pos >= WAV_CHUNK_HEADER) {

            u32 chunk_id   = 0;
            u32 chunk_size = 0;
            memcpy(&chunk_id,   data + pos,     sizeof(u32));
            memcpy(&chunk_size, data + pos + 4, sizeof(u32));
            pos += WAV_CHUNK_HEADER;

            // the chunk is truncated or corrupt
            if (chunk_size > length - pos) return(NULL);

            if (chunk_id == WAV_CSTR_FMT_) {
                if (chunk_size < sizeof(wav_format)) return(NULL);
                memcpy(&format, data + pos, sizeof(wav_format));
                has_fmt = true;
            }
            else if (chunk_id == WAV_CSTR_DATA) {
                samples   = data + pos;
                data_size = chunk_size;
            }

            // odd-sized chunks have a padding byte
            const u32 advance = chunk_size + (chunk_size & 1);
            if (advance >= length - pos) break;
            pos += advance;
        }
        if (!has_fmt || !samples) return(NULL);

        // validate the format
        bool is_valid = true;
        is_valid &= (format.audio_format    == WAV_FORMAT_PCM);
        is_valid &= (format.num_channels    != 0);
        is_valid &= (format.sample_rate     != 0);
        is_valid &= (format.bits_per_sample != 0);
        is_valid &= (format.block_align     != 0);
        is_valid &= (
            format.bits_per_sample == 8  ||
            format.bits_per_sample == 16 ||
            format.bits_per_sample == 24 ||
            format.bits_per_sample == 32
        );
        is_valid &= (format.block_align == (format.num_channels * format.bits_per_sample) / 8);
        is_valid &= (data_size >= format.block_align);
        if (!is_valid) return(NULL);

        // save the position in the arena
        const u32 save = arena_save(h_arena);

        // allocate the wav
        auto w = (wav*)arena_push(h_arena, sizeof(wav));
        if (!w) return(NULL);

        // allocate the wav data
        w->data.bytes = (byte*)arena_push(h_arena, data_size);
        if (w->data.bytes == NULL) {
            arena_revert(h_arena, save);
            return(NULL);
        }

        // copy the wav data
        w->format    = format;
        w->data.size = data_size;
        (void)memcpy((void*)w->data.bytes, samples, data_size);

        return(w);
    }

    // converts one sample to a float from -1 to 1
    // the samples aren't aligned, so they're copied out instead of cast
    IFB_INTERNAL f32
    wav_read_sample(
        const wav* w,
        const u32  frame,
        const u32  channel) {

        assert(w && channel < w->format.num_channels);

        const u32 sample_size = (w->format.bits_per_sample / 8);
        const u32 offset      = (frame * w->format.block_align) + (channel * sample_size);
        assert(offset + sample_size <= w->data.size);

        const byte* sample = w->data.bytes + offset;

        switch (w->format.bits_per_sample) {

            // 8 bit is the only unsigned one
            case (8): {
                return(((f32)sample[0] - 128.0f) / 128.0f);
            }
            case (16): {
                s16 value = 0;
                memcpy(&value, sample, sizeof(s16));
                return((f32)value / 32768.0f);
            }
            // the bytes go in the top of a u32 so the shift back down keeps the sign
            case (24): {
                const u32 bits = ((u32)sample[0] << 8) | ((u32)sample[1] << 16) | ((u32)sample[2] << 24);
                return((f32)(((s32)bits) >> 8) / 8388608.0f);
            }
            case (32): {
                s32 value = 0;
                memcpy(&value, sample, sizeof(s32));
                return((f32)value / 2147483648.0f);
            }
        }
        return(0.0f);
    }

};
