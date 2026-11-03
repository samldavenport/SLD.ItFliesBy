#pragma once

#include <cassert>

#include "eng-audio.hpp"
#include "eng-context.hpp"
#include "eng-memory.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "memory-reservation.cpp"
#include "eng-wav.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // a handle is the generation in the top 16 bits and the index in the bottom 16
    // the generation goes up every time the voice is freed, so old handles stop matching
    static constexpr u32 AUDIO_VOICE_INDEX_MASK       = 0xFFFF;
    static constexpr u32 AUDIO_VOICE_GENERATION_SHIFT = 16;

    static_assert(IFB_CONFIG_AUDIO_VOICE_CAPACITY <= AUDIO_VOICE_INDEX_MASK, "the voice index doesn't fit in a handle");

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the position and gain carry over between frames, a jump in either is an audible click
    // the gain fades toward the volume, or toward 0 when the voice is paused or stopping
    struct audio_voice {
        const wav* w;
        f64        position;   // in frames of the wav
        f64        step;       // wav frames per device frame
        u32        frame_count;
        f32        volume;
        f32        gain;
        u16        generation;
        bool       is_active;
        bool       is_paused;
        bool       is_looping;
        bool       is_stopping;
    };

    struct audio_mngr {
        audio_voice* voices;
        u32          voice_count;
        f32*  samples;
        u32   frame_capacity;    // the size of the samples buffer
        u32   frame_count;       // the frames to write this frame
        u32   latency_frames;    // the frames we keep queued on the device
        u64   timestamp_prev;
        bool  is_device_ready;
        bool  is_frame_open;     // the buffer is only valid between frame start and end
        bool  was_starved;       // only counts one underrun for each time the device runs dry
        struct {
            u32 frames_queued;
            u32 frames_queued_min;
            u32 underrun_count;
            f32 frame_gap_ms;
            f32 frame_gap_ms_max;
        } stats;
    } static * _audio_mngr;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline audio_voice* audio_mngr_voice_from_handle (const hnd_audio_voice h_voice);
    inline void         audio_mngr_voice_free        (audio_voice* voice);
    inline void         audio_mngr_voice_mix         (audio_voice* voice, const audio_buffer& buffer);
    inline void         audio_mngr_mix_voices        (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL audio_mngr*
    audio_mngr_create(
        void) {

        _audio_mngr = (audio_mngr*)global_alloc(sizeof(audio_mngr));
        assert(_audio_mngr);

        return(_audio_mngr);
    }

    IFB_INTERNAL void
    audio_mngr_startup(
        reservation* res) {

        assert(_audio_mngr);

        const u32 channel_count = config::audio_channel_count;
        const u32 sample_count  = config::audio_buffer_frames * channel_count;

        _audio_mngr->samples = (f32*)reservation_push_bytes(res, sample_count * sizeof(f32));
        assert(_audio_mngr->samples);

        const u32 voices_size = config::audio_voice_capacity * sizeof(audio_voice);
        _audio_mngr->voices   = (audio_voice*)reservation_push_bytes(res, voices_size);
        assert(_audio_mngr->voices);
        memset(_audio_mngr->voices, 0, voices_size);
        _audio_mngr->voice_count = 0;

        _audio_mngr->frame_capacity = config::audio_buffer_frames;
        _audio_mngr->latency_frames = config::audio_latency_frames;

        pfm_audio_config cfg;
        cfg.sample_rate   = config::audio_sample_rate;
        cfg.channel_count = channel_count;
        cfg.buffer_ms     = config::audio_buffer_ms;

        _audio_mngr->is_device_ready = pfm_audio_init(cfg);
        audio_mngr_reset_stats();
    }

    IFB_INTERNAL void
    audio_mngr_shutdown(
        void) {

        assert(_audio_mngr);

        pfm_audio_shutdown();
        _audio_mngr->is_device_ready = false;
    }

    IFB_INTERNAL void
    audio_mngr_frame_start(
        void) {

        assert(_audio_mngr);

        _audio_mngr->frame_count   = 0;
        _audio_mngr->is_frame_open = false;

        if (!_audio_mngr->is_device_ready) return;

        u32 frames_queued = 0;
        if (!pfm_audio_get_frames_queued(frames_queued)) {
            _audio_mngr->is_device_ready = false;
            return;
        }

        // the time since the last frame, this is how long the device had to live off of its queue
        const u64 timestamp = pfm_system_timestamp();
        auto&     stats     = _audio_mngr->stats;
        if (_audio_mngr->timestamp_prev != 0) {
            const u64 ticks    = timestamp - _audio_mngr->timestamp_prev;
            stats.frame_gap_ms = (f32)((f64)ticks * 1000.0 / (f64)pfm_system_frequency());
            if (stats.frame_gap_ms > stats.frame_gap_ms_max) {
                stats.frame_gap_ms_max = stats.frame_gap_ms;
            }

            if (frames_queued < stats.frames_queued_min) {
                stats.frames_queued_min = frames_queued;
            }

            // the queue only goes up when we write, so if it ran dry since the last frame it's still empty now
            if (frames_queued == 0 && !_audio_mngr->was_starved) {
                ++stats.underrun_count;
            }
        }
        _audio_mngr->timestamp_prev = timestamp;
        _audio_mngr->was_starved    = (frames_queued == 0);
        stats.frames_queued         = frames_queued;

        u32 frame_count = 0;
        if (frames_queued < _audio_mngr->latency_frames) {
            frame_count = _audio_mngr->latency_frames - frames_queued;
        }

        const u32 frames_free = pfm_audio_get_frame_capacity() - frames_queued;
        if (frame_count > frames_free)                 frame_count = frames_free;
        if (frame_count > _audio_mngr->frame_capacity) frame_count = _audio_mngr->frame_capacity;

        // silence is what plays if nothing writes this frame
        const u32 size = frame_count * config::audio_channel_count * sizeof(f32);
        memset(_audio_mngr->samples, 0, size);

        _audio_mngr->frame_count   = frame_count;
        _audio_mngr->is_frame_open = true;
    }

    IFB_INTERNAL void
    audio_mngr_frame_end(
        void) {

        assert(_audio_mngr);

        if (!_audio_mngr->is_frame_open) return;

        // after the callbacks, so the voices are added to anything they wrote
        audio_mngr_mix_voices();

        _audio_mngr->is_frame_open = false;

        const bool did_write = pfm_audio_write(_audio_mngr->samples, _audio_mngr->frame_count);
        if (!did_write) _audio_mngr->is_device_ready = false;
    }

    IFB_INTERNAL bool
    audio_mngr_get_buffer(
        audio_buffer& buffer) {

        assert(_audio_mngr);

        if (!_audio_mngr->is_frame_open) return(false);

        buffer.samples       = _audio_mngr->samples;
        buffer.frame_count   = _audio_mngr->frame_count;
        buffer.channel_count = config::audio_channel_count;
        buffer.sample_rate   = config::audio_sample_rate;
        return(true);
    }

    IFB_INTERNAL void
    audio_mngr_get_info(
        audio_info& info) {

        assert(_audio_mngr);

        const auto& stats = _audio_mngr->stats;

        info.is_device_ready   = _audio_mngr->is_device_ready;
        info.sample_rate       = config::audio_sample_rate;
        info.channel_count     = config::audio_channel_count;
        info.latency_frames    = _audio_mngr->latency_frames;
        info.frames_queued     = stats.frames_queued;
        info.frames_queued_min = stats.frames_queued_min;
        info.underrun_count    = stats.underrun_count;
        info.frame_gap_ms      = stats.frame_gap_ms;
        info.frame_gap_ms_max  = stats.frame_gap_ms_max;
        info.voice_count       = _audio_mngr->voice_count;
        info.voice_capacity    = config::audio_voice_capacity;
    }

    IFB_INTERNAL void
    audio_mngr_reset_stats(
        void) {

        assert(_audio_mngr);

        auto& stats = _audio_mngr->stats;
        stats.underrun_count    = 0;
        stats.frame_gap_ms_max  = 0.0f;
        stats.frames_queued_min = _audio_mngr->latency_frames;
    }

    // returns an invalid handle if there's no device, since nothing would mix the voice, or if every voice is in use
    IFB_INTERNAL hnd_audio_voice
    audio_mngr_voice_play(
        const wav* w,
        const f32  volume,
        const bool is_looping) {

        assert(_audio_mngr && w);

        hnd_audio_voice h_voice = {INVALID_HANDLE};
        if (!_audio_mngr->is_device_ready) return(h_voice);

        for (
            u32 index = 0;
                index < config::audio_voice_capacity;
              ++index) {

            audio_voice& voice = _audio_mngr->voices[index];
            if (voice.is_active) continue;

            // the generation is kept, it's what makes the old handles stale
            voice.w           = w;
            voice.position    = 0.0;
            voice.step        = (f64)w->format.sample_rate / (f64)config::audio_sample_rate;
            voice.frame_count = (w->data.size / w->format.block_align);
            voice.volume      = volume;
            voice.gain        = 0.0f;
            voice.is_active   = true;
            voice.is_paused   = false;
            voice.is_looping  = is_looping;
            voice.is_stopping = false;
            ++_audio_mngr->voice_count;

            h_voice.val = ((u32)voice.generation << AUDIO_VOICE_GENERATION_SHIFT) | index;
            return(h_voice);
        }
        return(h_voice);
    }

    IFB_INTERNAL void
    audio_mngr_voice_stop(
        const hnd_audio_voice h_voice) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (voice) voice->is_stopping = true;
    }

    IFB_INTERNAL void
    audio_mngr_voice_stop_now(
        const hnd_audio_voice h_voice) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (voice) audio_mngr_voice_free(voice);
    }

    // includes the voices that are fading out, which still read the wav
    IFB_INTERNAL void
    audio_mngr_voice_stop_all_using(
        const wav* w) {

        assert(_audio_mngr);

        for (
            u32 index = 0;
                index < config::audio_voice_capacity;
              ++index) {

            audio_voice& voice = _audio_mngr->voices[index];
            if (voice.is_active && voice.w == w) {
                audio_mngr_voice_free(&voice);
            }
        }
    }

    IFB_INTERNAL void
    audio_mngr_voice_set_paused(
        const hnd_audio_voice h_voice,
        const bool            is_paused) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (voice) voice->is_paused = is_paused;
    }

    IFB_INTERNAL void
    audio_mngr_voice_set_volume(
        const hnd_audio_voice h_voice,
        const f32             volume) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (voice) voice->volume = volume;
    }

    IFB_INTERNAL void
    audio_mngr_voice_set_looping(
        const hnd_audio_voice h_voice,
        const bool            is_looping) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (voice) voice->is_looping = is_looping;
    }

    // jumping while it's audible can click once
    IFB_INTERNAL void
    audio_mngr_voice_seek(
        const hnd_audio_voice h_voice,
        const f64             position) {

        audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (!voice) return;

        const f64 position_max = (f64)voice->frame_count;
        voice->position = (position < 0.0)          ? 0.0
                        : (position > position_max) ? position_max
                        : position;
    }

    IFB_INTERNAL bool
    audio_mngr_voice_get_info(
        const hnd_audio_voice h_voice,
        audio_voice_info&     info) {

        const audio_voice* voice = audio_mngr_voice_from_handle(h_voice);
        if (!voice) return(false);

        info.position    = voice->position;
        info.frame_count = voice->frame_count;
        info.sample_rate = voice->w->format.sample_rate;
        info.volume      = voice->volume;
        info.is_paused   = voice->is_paused;
        info.is_looping  = voice->is_looping;
        info.is_stopping = voice->is_stopping;
        return(true);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // NULL if the handle is invalid or stale
    inline audio_voice*
    audio_mngr_voice_from_handle(
        const hnd_audio_voice h_voice) {

        assert(_audio_mngr);

        if (h_voice == INVALID_HANDLE) return(NULL);

        const u32 index      = (h_voice.val & AUDIO_VOICE_INDEX_MASK);
        const u32 generation = (h_voice.val >> AUDIO_VOICE_GENERATION_SHIFT);
        if (index >= config::audio_voice_capacity) return(NULL);

        audio_voice* voice = &_audio_mngr->voices[index];
        const bool   is_match = (voice->is_active && voice->generation == generation);
        return(is_match ? voice : NULL);
    }

    inline void
    audio_mngr_voice_free(
        audio_voice* voice) {

        assert(voice->is_active && _audio_mngr->voice_count > 0);

        voice->is_active = false;
        voice->w         = NULL;
        ++voice->generation;
        --_audio_mngr->voice_count;
    }

    inline void
    audio_mngr_voice_mix(
        audio_voice*        voice,
        const audio_buffer& buffer) {

        const wav*        w           = voice->w;
        const u32         channels    = w->format.num_channels;
        const bool        is_audible  = (!voice->is_paused && !voice->is_stopping);
        const f32         gain_target = is_audible ? voice->volume : 0.0f;
        const f32         gain_step   = 1.0f / (f32)config::audio_fade_frames;

        for (
            u32 i_frame = 0;
                i_frame < buffer.frame_count;
              ++i_frame) {

            if      (voice->gain < gain_target) { voice->gain += gain_step; if (voice->gain > gain_target) voice->gain = gain_target; }
            else if (voice->gain > gain_target) { voice->gain -= gain_step; if (voice->gain < gain_target) voice->gain = gain_target; }

            // the fade out is done, a paused voice holds its position until it's resumed
            if (!is_audible && voice->gain <= 0.0f) break;

            // the end of the wav
            if (voice->position >= (f64)voice->frame_count) {
                if (!voice->is_looping) {
                    audio_mngr_voice_free(voice);
                    return;
                }
                voice->position -= (f64)voice->frame_count;
            }

            // the wav's sample rate may not match the device, so this blends the 2 closest frames
            const u32 frame_a = (u32)voice->position;
            const f32 blend   = (f32)(voice->position - (f64)frame_a);
            u32       frame_b = frame_a + 1;
            if (frame_b >= voice->frame_count) {
                frame_b = voice->is_looping ? 0 : frame_a;
            }

            for (
                u32 i_channel = 0;
                    i_channel < buffer.channel_count;
                  ++i_channel) {

                // mono goes to every channel, extra channels in the wav are ignored
                const u32 wav_channel = (i_channel < channels) ? i_channel : (channels - 1);

                const f32 sample_a = wav_read_sample(w, frame_a, wav_channel);
                const f32 sample_b = wav_read_sample(w, frame_b, wav_channel);

                buffer.samples[(i_frame * buffer.channel_count) + i_channel] += (sample_a + ((sample_b - sample_a) * blend)) * voice->gain;
            }

            voice->position += voice->step;
        }

        if (voice->is_stopping && voice->gain <= 0.0f) {
            audio_mngr_voice_free(voice);
        }
    }

    // the voices add to the buffer, then the sum is clamped once so a loud mix clips instead of wrapping
    inline void
    audio_mngr_mix_voices(
        void) {

        audio_buffer buffer;
        if (!audio_mngr_get_buffer(buffer) || buffer.frame_count == 0) return;

        if (_audio_mngr->voice_count != 0) {
            for (
                u32 index = 0;
                    index < config::audio_voice_capacity;
                  ++index) {

                audio_voice* voice = &_audio_mngr->voices[index];
                if (voice->is_active) audio_mngr_voice_mix(voice, buffer);
            }
        }

        const u32 sample_count = buffer.frame_count * buffer.channel_count;
        for (
            u32 i_sample = 0;
                i_sample < sample_count;
              ++i_sample) {

            f32& sample = buffer.samples[i_sample];
            if      (sample >  1.0f) sample =  1.0f;
            else if (sample < -1.0f) sample = -1.0f;
        }
    }
};
