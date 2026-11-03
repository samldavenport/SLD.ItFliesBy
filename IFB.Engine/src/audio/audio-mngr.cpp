#pragma once

#include <cassert>
#include <math.h>

#include "eng-audio.hpp"
#include "eng-context.hpp"
#include "eng-memory.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "memory-reservation.cpp"
#include "eng-wav.hpp"
#include "eng-asset.hpp"
#include "audio-source.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // a handle is the generation in the top 16 bits and the index in the bottom 16
    // the generation goes up every time the voice is freed, so old handles stop matching
    static constexpr u32 AUDIO_VOICE_INDEX_MASK       = 0xFFFF;
    static constexpr u32 AUDIO_VOICE_GENERATION_SHIFT = 16;

    static_assert(IFB_CONFIG_AUDIO_VOICE_CAPACITY <= AUDIO_VOICE_INDEX_MASK, "the voice index doesn't fit in a handle");

    // a voice is mixed a block of device frames at a time, the source frames for a block are read into the scratch buffer first
    // a source at a higher sample rate needs more of its frames for a block, so its blocks are shorter to fit
    static constexpr u32 AUDIO_MIX_BLOCK_FRAMES   = 256;
    static constexpr u32 AUDIO_SCRATCH_FRAMES     = 1024;
    static constexpr u32 AUDIO_SCRATCH_CHANNELS   = 2;
    static constexpr u32 AUDIO_SCRATCH_PAD_FRAMES = 4;   // the frame after the last one, for blending, plus room for rounding

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the position and gain carry over between frames, a jump in either is an audible click
    // the gain fades toward the volume, or toward 0 when the voice is paused or stopping
    // the source has the samples, a voice playing a sound asset has it pinned through its source
    struct audio_voice {
        audio_source source;
        f64          position;   // in frames of the source
        f64          step;       // source frames per device frame
        u32          group;
        f32        volume;
        f32        gain;       // fades toward volume times the group's volume
        u16        generation;
        bool       is_active;
        bool       is_paused;
        bool       is_looping;
        bool       is_stopping;
    };

    // a preloaded sound has one pin however many times it's preloaded, the count is the unloads it takes to let it go
    // it's unpinned while the runtime database is replaced, and pinned again from the new one
    struct audio_preload {
        hnd_asset hnd;
        u32       count;     // 0 if the entry is free
        bool      is_pinned;
    };

    struct audio_mngr {
        f32            group_volumes[audio_group_e_count];
        f32            master_volume;
        f32            master_gain;     // fades toward master_volume, the same as a voice's gain
        audio_voice*   voices;
        u32            voice_count;
        audio_preload* preloads;
        u32            preload_count;
        f32*           scratch;         // AUDIO_SCRATCH_FRAMES frames of AUDIO_SCRATCH_CHANNELS, the source frames for one block
        audio_stream*  streams;
        u32            stream_count;
        u32            stream_chunk_reads; // since the last reset
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
    inline hnd_audio_voice audio_mngr_voice_start    (const audio_source& src, const audio_group_e group, const f32 volume, const bool is_looping);
    inline void         audio_mngr_voice_fetch       (const audio_voice* voice, const u32 frame_first, const u32 count, f32* out);
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

        for (
            u32 group = 0;
                group < audio_group_e_count;
              ++group) {

            _audio_mngr->group_volumes[group] = 1.0f;
        }
        _audio_mngr->master_volume = 1.0f;
        _audio_mngr->master_gain   = 1.0f;

        const u32 preloads_size = config::audio_preload_capacity * sizeof(audio_preload);
        _audio_mngr->preloads   = (audio_preload*)reservation_push_bytes(res, preloads_size);
        assert(_audio_mngr->preloads);
        memset(_audio_mngr->preloads, 0, preloads_size);
        _audio_mngr->preload_count = 0;

        const u32 scratch_size = AUDIO_SCRATCH_FRAMES * AUDIO_SCRATCH_CHANNELS * sizeof(f32);
        _audio_mngr->scratch   = (f32*)reservation_push_bytes(res, scratch_size);
        assert(_audio_mngr->scratch);

        // the chunks are set aside up front, so starting a stream never allocates
        const u32 streams_size      = config::audio_stream_capacity * sizeof(audio_stream);
        const u32 chunk_size        = AUDIO_STREAM_CHUNK_FRAMES * AUDIO_STREAM_CHANNELS_MAX * sizeof(s16);
        const u32 chunk_memory_size = config::audio_stream_capacity * AUDIO_STREAM_CHUNK_COUNT * chunk_size;
        _audio_mngr->streams        = (audio_stream*)reservation_push_bytes(res, streams_size);
        byte* chunk_memory          = (byte*)reservation_push_bytes(res, chunk_memory_size);
        assert(_audio_mngr->streams && chunk_memory);
        memset(_audio_mngr->streams, 0, streams_size);
        for (
            u32 i_stream = 0;
                i_stream < config::audio_stream_capacity;
              ++i_stream) {

            for (
                u32 i_chunk = 0;
                    i_chunk < AUDIO_STREAM_CHUNK_COUNT;
                  ++i_chunk) {

                const u32 offset = ((i_stream * AUDIO_STREAM_CHUNK_COUNT) + i_chunk) * chunk_size;
                _audio_mngr->streams[i_stream].chunks[i_chunk].samples = (s16*)(chunk_memory + offset);
                _audio_mngr->streams[i_stream].chunks[i_chunk].index   = INVALID_INDEX;
            }
        }
        _audio_mngr->stream_count       = 0;
        _audio_mngr->stream_chunk_reads = 0;

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

        // the voices go first, so the sounds they pinned are unpinned before the assets shut down
        for (
            u32 index = 0;
                index < config::audio_voice_capacity;
              ++index) {

            audio_voice& voice = _audio_mngr->voices[index];
            if (voice.is_active) audio_mngr_voice_free(&voice);
        }

        // and the preloads let go of theirs
        for (
            u32 index = 0;
                index < config::audio_preload_capacity;
              ++index) {

            audio_preload& preload = _audio_mngr->preloads[index];
            if (preload.is_pinned) asset_mngr_unpin(preload.hnd);
            preload = {};
        }
        _audio_mngr->preload_count = 0;

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
        info.preload_count      = _audio_mngr->preload_count;
        info.preload_capacity   = config::audio_preload_capacity;
        info.stream_count       = _audio_mngr->stream_count;
        info.stream_capacity    = config::audio_stream_capacity;
        info.stream_chunk_reads = _audio_mngr->stream_chunk_reads;
    }

    IFB_INTERNAL void
    audio_mngr_reset_stats(
        void) {

        assert(_audio_mngr);

        auto& stats = _audio_mngr->stats;
        stats.underrun_count    = 0;
        stats.frame_gap_ms_max  = 0.0f;
        stats.frames_queued_min = _audio_mngr->latency_frames;
        _audio_mngr->stream_chunk_reads = 0;
    }

    // the voices fade to it, since the mixer reads it for the gain they fade toward
    IFB_INTERNAL void
    audio_mngr_group_set_volume(
        const audio_group_e group,
        const f32           volume) {

        assert(_audio_mngr && group < audio_group_e_count);

        _audio_mngr->group_volumes[group] = (volume < 0.0f) ? 0.0f
                                          : (volume > 1.0f) ? 1.0f
                                          : volume;
    }

    // the whole buffer is scaled at the end of the frame, so it covers everything written to it, not just the voices
    IFB_INTERNAL void
    audio_mngr_set_master_volume(
        const f32 volume) {

        assert(_audio_mngr);

        _audio_mngr->master_volume = (volume < 0.0f) ? 0.0f
                                   : (volume > 1.0f) ? 1.0f
                                   : volume;
    }

    IFB_INTERNAL f32
    audio_mngr_get_master_volume(
        void) {

        assert(_audio_mngr);
        return(_audio_mngr->master_volume);
    }

    IFB_INTERNAL f32
    audio_mngr_group_get_volume(
        const audio_group_e group) {

        assert(_audio_mngr && group < audio_group_e_count);
        return(_audio_mngr->group_volumes[group]);
    }

    // returns an invalid handle if there's no device, since nothing would mix the voice, or if every voice is in use
    IFB_INTERNAL hnd_audio_voice
    audio_mngr_voice_play(
        const wav*          w,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        assert(_audio_mngr && w);

        const audio_source src = audio_source_from_memory(w, {INVALID_HANDLE});
        return(audio_mngr_voice_start(src, group, volume, is_looping));
    }

    // loads the sound from the runtime database if it isn't in memory, and keeps it pinned while the voice plays
    // returns an invalid handle if there's no sound by that name, no room for it, no device, or every voice is in use
    IFB_INTERNAL hnd_audio_voice
    audio_mngr_voice_play_sound(
        const cchar*        name,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        assert(_audio_mngr && name);

        hnd_audio_voice h_voice = {INVALID_HANDLE};
        if (!_audio_mngr->is_device_ready) return(h_voice);

        const hnd_asset h_sound = asset_mngr_load_sound(name);
        if (h_sound == INVALID_HANDLE) return(h_voice);

        const asset_sound* snd = asset_mngr_pin_sound(h_sound);
        if (!snd) return(h_voice);

        // the source keeps the pin, and lets go of it when the voice is freed
        audio_source src = audio_source_from_memory(&snd->w, h_sound);
        h_voice = audio_mngr_voice_start(src, group, volume, is_looping);
        if (h_voice == INVALID_HANDLE) {
            audio_source_release(src);
        }
        return(h_voice);
    }

    // the sound is read from the bank a chunk at a time while it plays, instead of being loaded
    // the first chunks are read now, so it starts right away
    // returns an invalid handle if there's no sound by that name, no bank, no device, or every stream or voice is in use
    IFB_INTERNAL hnd_audio_voice
    audio_mngr_voice_stream_sound(
        const cchar*        name,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        assert(_audio_mngr && name);

        hnd_audio_voice h_voice = {INVALID_HANDLE};
        if (!_audio_mngr->is_device_ready) return(h_voice);

        const hnd_asset h_sound = asset_name_cstr_to_handle(name);
        if (h_sound == INVALID_HANDLE) return(h_voice);

        asset_sound_stream_info info;
        if (!asset_mngr_get_sound_stream_info(h_sound, info)) return(h_voice);

        audio_stream* stream = NULL;
        for (
            u32 i_stream = 0;
                i_stream < config::audio_stream_capacity && !stream;
              ++i_stream) {

            if (!_audio_mngr->streams[i_stream].is_active) stream = &_audio_mngr->streams[i_stream];
        }
        if (!stream) return(h_voice);

        audio_stream_open(stream, h_sound, info);
        const u32 reads_before = stream->chunk_reads;
        audio_stream_refill(stream, 0.0, is_looping);
        _audio_mngr->stream_chunk_reads += (stream->chunk_reads - reads_before);

        // the voice takes the stream, and gives it back when it's freed
        audio_source src = audio_source_from_stream(stream);
        h_voice = audio_mngr_voice_start(src, group, volume, is_looping);
        if (h_voice == INVALID_HANDLE) {
            audio_source_release(src);
            return(h_voice);
        }

        ++_audio_mngr->stream_count;
        return(h_voice);
    }

    // frees every voice playing a sound asset without a fade, so nothing is pinned or reading the bank
    IFB_INTERNAL void
    audio_mngr_voice_stop_all_sounds(
        void) {

        assert(_audio_mngr);

        for (
            u32 index = 0;
                index < config::audio_voice_capacity;
              ++index) {

            audio_voice& voice = _audio_mngr->voices[index];
            const bool is_sound = (voice.source.h_sound != INVALID_HANDLE || voice.source.type == audio_source_type_e_stream);
            if (voice.is_active && is_sound) {
                audio_mngr_voice_free(&voice);
            }
        }
    }

    // loads the sound if it isn't in memory, and keeps it there until it's unloaded as many times as it was preloaded
    // false if there's no sound by that name, no room for it, or every preload is in use
    IFB_INTERNAL bool
    audio_mngr_sound_preload(
        const cchar* name) {

        assert(_audio_mngr && name);

        const hnd_asset h_sound = asset_mngr_load_sound(name);
        if (h_sound == INVALID_HANDLE) return(false);

        // it may already be preloaded
        audio_preload* entry_free = NULL;
        for (
            u32 index = 0;
                index < config::audio_preload_capacity;
              ++index) {

            audio_preload& preload = _audio_mngr->preloads[index];
            if (preload.count == 0) {
                if (!entry_free) entry_free = &preload;
                continue;
            }
            if (preload.hnd == h_sound) {
                ++preload.count;
                return(true);
            }
        }
        if (!entry_free) return(false);

        if (!asset_mngr_pin_sound(h_sound)) return(false);

        entry_free->hnd       = h_sound;
        entry_free->count     = 1;
        entry_free->is_pinned = true;
        ++_audio_mngr->preload_count;
        return(true);
    }

    // the last unload unpins it, so it can be evicted, it isn't freed right away
    // a sound that isn't preloaded is ignored
    IFB_INTERNAL void
    audio_mngr_sound_unload(
        const cchar* name) {

        assert(_audio_mngr && name);

        const hnd_asset h_sound = asset_name_cstr_to_handle(name);
        if (h_sound == INVALID_HANDLE) return;

        for (
            u32 index = 0;
                index < config::audio_preload_capacity;
              ++index) {

            audio_preload& preload = _audio_mngr->preloads[index];
            if (preload.count == 0 || preload.hnd != h_sound) continue;

            --preload.count;
            if (preload.count == 0) {
                if (preload.is_pinned) asset_mngr_unpin(preload.hnd);
                preload = {};
                --_audio_mngr->preload_count;
            }
            return;
        }
    }

    // the runtime database can't close while anything is pinned
    // the preloads are kept, so they can be pinned again once it's open
    IFB_INTERNAL void
    audio_mngr_sounds_release(
        void) {

        assert(_audio_mngr);

        audio_mngr_voice_stop_all_sounds();

        for (
            u32 index = 0;
                index < config::audio_preload_capacity;
              ++index) {

            audio_preload& preload = _audio_mngr->preloads[index];
            if (!preload.is_pinned) continue;

            asset_mngr_unpin(preload.hnd);
            preload.is_pinned = false;
        }
    }

    // a preload whose sound isn't in the new database, or doesn't fit, is dropped
    IFB_INTERNAL void
    audio_mngr_sounds_reacquire(
        void) {

        assert(_audio_mngr);

        for (
            u32 index = 0;
                index < config::audio_preload_capacity;
              ++index) {

            audio_preload& preload = _audio_mngr->preloads[index];
            if (preload.count == 0 || preload.is_pinned) continue;

            preload.is_pinned = (asset_mngr_pin_sound(preload.hnd) != NULL);
            if (!preload.is_pinned) {
                preload = {};
                --_audio_mngr->preload_count;
            }
        }
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
            if (voice.is_active && voice.source.type == audio_source_type_e_memory && voice.source.w == w) {
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

        const f64 position_max = (f64)voice->source.frame_count;
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
        info.frame_count = voice->source.frame_count;
        info.sample_rate = voice->source.sample_rate;
        info.volume      = voice->volume;
        info.group       = voice->group;
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

        if (voice->source.type == audio_source_type_e_stream) {
            assert(_audio_mngr->stream_count > 0);
            --_audio_mngr->stream_count;
        }
        audio_source_release(voice->source);

        voice->is_active = false;
        ++voice->generation;
        --_audio_mngr->voice_count;
    }

    // returns an invalid handle if there's no device, since nothing would mix the voice, or if every voice is in use
    // the voice takes over the source, including its pin, only if it starts
    inline hnd_audio_voice
    audio_mngr_voice_start(
        const audio_source& src,
        const audio_group_e group,
        const f32           volume,
        const bool          is_looping) {

        assert(group < audio_group_e_count);
        assert(src.type != audio_source_type_e_none && src.frame_count != 0 && src.sample_rate != 0);

        hnd_audio_voice h_voice = {INVALID_HANDLE};
        if (!_audio_mngr->is_device_ready) return(h_voice);

        for (
            u32 index = 0;
                index < config::audio_voice_capacity;
              ++index) {

            audio_voice& voice = _audio_mngr->voices[index];
            if (voice.is_active) continue;

            // the generation is kept, it's what makes the old handles stale
            voice.source      = src;
            voice.position    = 0.0;
            voice.step        = (f64)src.sample_rate / (f64)config::audio_sample_rate;
            voice.group       = group;
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

    // reads count frames from frame_first on, where the frames can run past the end of the source
    // a looping voice wraps back to the start, any number of times for a short source
    // a voice that isn't looping holds its last frame, so the end blends into it the same as it always did
    // a source that comes up short is filled out with silence
    inline void
    audio_mngr_voice_fetch(
        const audio_voice* voice,
        const u32          frame_first,
        const u32          count,
        f32*               out) {

        const audio_source& src         = voice->source;
        const u32           frame_count = src.frame_count;
        const u32           channels    = src.channel_count;

        u32 i_frame = 0;
        while (i_frame < count) {

            const u64 frame = (u64)frame_first + i_frame;
            u32       index = 0;
            u32       run   = 0;

            if (voice->is_looping) {
                index = (u32)(frame % frame_count);
                run   = frame_count - index;
            }
            else if (frame < frame_count) {
                index = (u32)frame;
                run   = frame_count - index;
            }
            else {
                // past the end, every frame left is the last one
                const u32 last = (i_frame != 0) ? (i_frame - 1) : 0;
                if (i_frame == 0) {
                    (void)audio_source_read(src, frame_count - 1, 1, out);
                    ++i_frame;
                }
                for (
                    ;
                    i_frame < count;
                  ++i_frame) {

                    memcpy(out + (i_frame * channels), out + (last * channels), channels * sizeof(f32));
                }
                break;
            }

            if (run > count - i_frame) run = count - i_frame;

            const u32 did_read = audio_source_read(src, index, run, out + (i_frame * channels));
            if (did_read < run) {
                memset(out + ((i_frame + did_read) * channels), 0, (run - did_read) * channels * sizeof(f32));
            }
            i_frame += run;
        }
    }

    inline void
    audio_mngr_voice_mix(
        audio_voice*        voice,
        const audio_buffer& buffer) {

        // a stream reads the chunks this frame needs before it's mixed, a seek since the last frame included
        // a frame is shorter than a chunk, so the chunk under the playhead and the next one cover it
        if (voice->source.type == audio_source_type_e_stream) {
            audio_stream* stream       = voice->source.stream;
            const u32     reads_before = stream->chunk_reads;
            audio_stream_refill(stream, voice->position, voice->is_looping);
            _audio_mngr->stream_chunk_reads += (stream->chunk_reads - reads_before);
        }

        const audio_source& src         = voice->source;
        const u32           channels    = src.channel_count;
        const f64           frame_count = (f64)src.frame_count;
        const bool          is_audible  = (!voice->is_paused && !voice->is_stopping);
        const f32           gain_target = is_audible ? (voice->volume * _audio_mngr->group_volumes[voice->group]) : 0.0f;
        const f32           gain_step   = 1.0f / (f32)config::audio_fade_frames;
        f32*                scratch     = _audio_mngr->scratch;

        // the block has to fit its source frames in the scratch buffer
        const u32 step_ceil = (u32)voice->step + 1;
        u32       block_max = ((AUDIO_SCRATCH_FRAMES - AUDIO_SCRATCH_PAD_FRAMES) / step_ceil) + 1;
        if (block_max > AUDIO_MIX_BLOCK_FRAMES) block_max = AUDIO_MIX_BLOCK_FRAMES;

        bool is_done = false;
        u32  i_frame = 0;
        while (i_frame < buffer.frame_count && !is_done) {

            // a seek can put the position at the end, and a looping voice comes back around between blocks
            if (voice->position >= frame_count) {
                if (!voice->is_looping) {
                    audio_mngr_voice_free(voice);
                    return;
                }
                voice->position = fmod(voice->position, frame_count);
            }

            // the source frames for the whole block, the positions in the block don't wrap, the fetch does
            const u32 block_count = (buffer.frame_count - i_frame < block_max) ? (buffer.frame_count - i_frame) : block_max;
            const u32 frame_first = (u32)voice->position;
            const f64 position_end = voice->position + ((f64)(block_count - 1) * voice->step);
            const u32 fetch_count  = ((u32)position_end - frame_first) + AUDIO_SCRATCH_PAD_FRAMES;
            assert(fetch_count <= AUDIO_SCRATCH_FRAMES);
            audio_mngr_voice_fetch(voice, frame_first, fetch_count, scratch);

            f64 position = voice->position;
            for (
                u32 i_block = 0;
                    i_block < block_count;
                  ++i_block, ++i_frame) {

                if      (voice->gain < gain_target) { voice->gain += gain_step; if (voice->gain > gain_target) voice->gain = gain_target; }
                else if (voice->gain > gain_target) { voice->gain -= gain_step; if (voice->gain < gain_target) voice->gain = gain_target; }

                // the fade out is done, a paused voice holds its position until it's resumed
                if (!is_audible && voice->gain <= 0.0f) {
                    is_done = true;
                    break;
                }

                // the end of a voice that isn't looping
                if (!voice->is_looping && position >= frame_count) {
                    audio_mngr_voice_free(voice);
                    return;
                }

                // the source's sample rate may not match the device, so this blends the 2 closest frames
                const u32 frame_a = (u32)position;
                const f32 blend   = (f32)(position - (f64)frame_a);
                const u32 index_a = (frame_a - frame_first) * channels;
                const u32 index_b = index_a + channels;

                for (
                    u32 i_channel = 0;
                        i_channel < buffer.channel_count;
                      ++i_channel) {

                    // mono goes to every channel
                    const u32 src_channel = (i_channel < channels) ? i_channel : (channels - 1);
                    const f32 sample_a    = scratch[index_a + src_channel];
                    const f32 sample_b    = scratch[index_b + src_channel];

                    buffer.samples[(i_frame * buffer.channel_count) + i_channel] += (sample_a + ((sample_b - sample_a) * blend)) * voice->gain;
                }

                position += voice->step;
            }
            voice->position = position;
        }

        // a looping voice's position stays inside the source
        if (voice->is_looping && voice->position >= frame_count) {
            voice->position = fmod(voice->position, frame_count);
        }

        if (voice->is_stopping && voice->gain <= 0.0f) {
            audio_mngr_voice_free(voice);
        }
    }

    // the voices add to the buffer, then the sum is scaled by the master volume and clamped once, so a loud mix clips instead of wrapping
    // the master volume applies to anything written to the buffer directly too
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

        const f32 gain_target = _audio_mngr->master_volume;
        const f32 gain_step   = 1.0f / (f32)config::audio_fade_frames;
        f32&      gain        = _audio_mngr->master_gain;

        for (
            u32 i_frame = 0;
                i_frame < buffer.frame_count;
              ++i_frame) {

            if      (gain < gain_target) { gain += gain_step; if (gain > gain_target) gain = gain_target; }
            else if (gain > gain_target) { gain -= gain_step; if (gain < gain_target) gain = gain_target; }

            for (
                u32 i_channel = 0;
                    i_channel < buffer.channel_count;
                  ++i_channel) {

                f32& sample = buffer.samples[(i_frame * buffer.channel_count) + i_channel];
                sample *= gain;
                if      (sample >  1.0f) sample =  1.0f;
                else if (sample < -1.0f) sample = -1.0f;
            }
        }
    }
};
