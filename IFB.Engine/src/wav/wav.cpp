#pragma once

#include "eng-wav.hpp"
#include "eng-memory.hpp"

namespace ifb {

    // the samples aren't copied, w.data points into the file's data, so it has to outlive the wav
    IFB_INTERNAL bool
    wav_parse_in_place(
        const byte* data,
        const u32   length,
        wav&        w) {

        if (!data || length < WAV_RIFF_HEADER) return(false);

        // validate the riff header
        u32 riff_id = 0;
        u32 wave_id = 0;
        memcpy(&riff_id, data,     sizeof(u32));
        memcpy(&wave_id, data + 8, sizeof(u32));
        if (riff_id != WAV_CSTR_RIFF || wave_id != WAV_CSTR_WAVE) return(false);

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
            if (chunk_size > length - pos) return(false);

            if (chunk_id == WAV_CSTR_FMT_) {
                if (chunk_size < sizeof(wav_format)) return(false);
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
        if (!has_fmt || !samples) return(false);

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
        if (!is_valid) return(false);

        w.format     = format;
        w.data.bytes = (byte*)samples;
        w.data.size  = data_size;
        return(true);
    }

    // the wav and a copy of its samples go in the arena, so the file's data can go away
    IFB_INTERNAL wav*
    wav_parse_data(
        const byte*     data,
        const u32       length,
        const hnd_arena h_arena) {

        assert(h_arena != INVALID_HANDLE);

        wav parsed;
        if (!wav_parse_in_place(data, length, parsed)) return(NULL);

        // save the position in the arena
        const u32 save = arena_save(h_arena);

        // allocate the wav
        auto w = (wav*)arena_push(h_arena, sizeof(wav));
        if (!w) return(NULL);

        // allocate the wav data
        *w            = parsed;
        w->data.bytes = (byte*)arena_push(h_arena, parsed.data.size);
        if (w->data.bytes == NULL) {
            arena_revert(h_arena, save);
            return(NULL);
        }

        // copy the wav data
        (void)memcpy((void*)w->data.bytes, parsed.data.bytes, parsed.data.size);

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

    // the frames the wav has once it's converted to sample_rate
    IFB_INTERNAL u32
    wav_convert_frame_count(
        const wav& w,
        const u32  sample_rate) {

        const u64 frame_count_in  = (w.data.size / w.format.block_align);
        const u64 frame_count_out = (frame_count_in * sample_rate) / w.format.sample_rate;
        return((frame_count_out != 0) ? (u32)frame_count_out : 1);
    }

    // the channels the wav has once it's converted, mono stays mono and everything else is stereo
    IFB_INTERNAL u32
    wav_convert_channel_count(
        const wav& w) {

        return((w.format.num_channels == 1) ? 1 : 2);
    }

    // converts to interleaved 16 bit samples at sample_rate, with wav_convert_channel_count channels
    // samples needs room for wav_convert_frame_count frames, past the first 2 channels are dropped
    // the rate is changed by blending the 2 closest frames
    IFB_INTERNAL void
    wav_convert_to_s16(
        const wav& w,
        const u32  sample_rate,
        s16*       samples) {

        assert(samples);

        const u32 frame_count_in  = (w.data.size / w.format.block_align);
        const u32 frame_count_out = wav_convert_frame_count(w, sample_rate);
        const u32 channel_count   = wav_convert_channel_count(w);
        const f64 step            = (f64)w.format.sample_rate / (f64)sample_rate;

        for (
            u32 i_frame = 0;
                i_frame < frame_count_out;
              ++i_frame) {

            const f64 position = (f64)i_frame * step;
            const u32 frame_a  = (u32)position;
            const f32 blend    = (f32)(position - (f64)frame_a);
            const u32 frame_b  = (frame_a + 1 < frame_count_in) ? (frame_a + 1) : frame_a;

            for (
                u32 i_channel = 0;
                    i_channel < channel_count;
                  ++i_channel) {

                const f32 sample_a = wav_read_sample(&w, frame_a, i_channel);
                const f32 sample_b = wav_read_sample(&w, frame_b, i_channel);
                const f32 sample   = sample_a + ((sample_b - sample_a) * blend);

                // rounded, and clamped since a full scale sample from a higher bit depth can round past the top
                f32 scaled = sample * 32768.0f;
                scaled = (scaled >= 0.0f) ? (scaled + 0.5f) : (scaled - 0.5f);
                if      (scaled >  32767.0f) scaled =  32767.0f;
                else if (scaled < -32768.0f) scaled = -32768.0f;

                samples[(i_frame * channel_count) + i_channel] = (s16)scaled;
            }
        }
    }
};
