#pragma once

#include <cassert>

#include "eng-audio.hpp"
#include "eng-context.hpp"
#include "eng-memory.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    struct audio_mngr {
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
};
