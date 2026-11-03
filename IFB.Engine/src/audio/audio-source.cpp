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
        audio_source_type_e_memory = 1,
        audio_source_type_e_stream = 2
    };

    static constexpr u32 AUDIO_STREAM_CHUNK_FRAMES = IFB_CONFIG_AUDIO_STREAM_CHUNK_FRAMES;
    static constexpr u32 AUDIO_STREAM_CHUNK_COUNT  = IFB_CONFIG_AUDIO_STREAM_CHUNK_COUNT;
    static constexpr u32 AUDIO_STREAM_CHANNELS_MAX = 2;

    // a sound is split into chunks of AUDIO_STREAM_CHUNK_FRAMES, and the last one is shorter
    // the samples are as they are in the bank, 16 bit with the sound's channels
    struct audio_stream_chunk {
        u32  index;       // which chunk of the sound, INVALID_INDEX if it's empty
        u32  frame_count;
        s16* samples;     // room for AUDIO_STREAM_CHUNK_FRAMES of AUDIO_STREAM_CHANNELS_MAX
    };

    // a ring of chunks, refilled before the voice is mixed so it holds the chunk under the playhead and the ones after it
    // nothing is pinned, the samples come from the bank as they're needed
    struct audio_stream {
        asset_sound_stream_info info;
        hnd_asset               h_sound;
        u32                     chunk_reads;  // every chunk read from the bank, for the stats
        bool                    is_active;
        audio_stream_chunk      chunks[AUDIO_STREAM_CHUNK_COUNT];
    };

    // the source gives the mixer at most 2 channels, a mono source stays mono and extra channels in the wav are dropped
    // a sound asset stays pinned for as long as the source has it, so its samples can't move
    struct audio_source {
        u32        type;
        u32        frame_count;
        u32        sample_rate;
        u32        channel_count; // 1 or 2, what the source gives the mixer
        const wav*    w;          // memory
        audio_stream* stream;     // stream
        hnd_asset     h_sound;    // INVALID_HANDLE if it isn't a sound asset, only a memory source has it pinned
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
        src.stream        = NULL;
        src.h_sound       = h_sound;
        return(src);
    }

    // the stream has to be open, the source keeps it until it's released
    inline audio_source
    audio_source_from_stream(
        audio_stream* stream) {

        assert(stream && stream->is_active);

        audio_source src;
        src.type          = audio_source_type_e_stream;
        src.frame_count   = stream->info.frame_count;
        src.sample_rate   = stream->info.sample_rate;
        src.channel_count = stream->info.channel_count;
        src.w             = NULL;
        src.stream        = stream;
        src.h_sound       = stream->h_sound;
        return(src);
    }

    // the chunks start out empty, so the first refill reads them
    inline void
    audio_stream_open(
        audio_stream*                  stream,
        const hnd_asset                h_sound,
        const asset_sound_stream_info& info) {

        assert(stream && !stream->is_active);
        assert(info.channel_count >= 1 && info.channel_count <= AUDIO_STREAM_CHANNELS_MAX && info.frame_count != 0);

        stream->info      = info;
        stream->h_sound   = h_sound;
        stream->is_active = true;
        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            stream->chunks[i_chunk].index       = INVALID_INDEX;
            stream->chunks[i_chunk].frame_count = 0;
        }
    }

    inline const audio_stream_chunk*
    audio_stream_find_chunk(
        const audio_stream* stream,
        const u32           index) {

        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            if (stream->chunks[i_chunk].index == index) return(&stream->chunks[i_chunk]);
        }
        return(NULL);
    }

    // makes sure the ring has the chunk the position is in and the ones after it, reading the missing ones from the bank
    // a looping stream's chunks after the last one are the first ones again, so a loop doesn't wait on the disk
    // a chunk that can't be read stays empty, and plays as silence
    inline void
    audio_stream_refill(
        audio_stream* stream,
        const f64     position,
        const bool    is_looping) {

        const u32 frame_count = stream->info.frame_count;
        const u32 chunk_total = (frame_count + AUDIO_STREAM_CHUNK_FRAMES - 1) / AUDIO_STREAM_CHUNK_FRAMES;

        u32 chunk_first = (u32)position / AUDIO_STREAM_CHUNK_FRAMES;
        if (chunk_first >= chunk_total) chunk_first = chunk_total - 1;

        // the chunks it needs, in the order they'll play, a short sound that loops needs fewer
        u32 needed[AUDIO_STREAM_CHUNK_COUNT];
        u32 needed_count = 0;
        for (
            u32 i_next = 0;
                i_next < AUDIO_STREAM_CHUNK_COUNT;
              ++i_next) {

            u32 index = chunk_first + i_next;
            if (index >= chunk_total) {
                if (!is_looping) break;
                index %= chunk_total;
            }

            bool is_listed = false;
            for (u32 i = 0; i < needed_count; ++i) is_listed |= (needed[i] == index);
            if (!is_listed) needed[needed_count++] = index;
        }

        for (
            u32 i_needed = 0;
                i_needed < needed_count;
              ++i_needed) {

            const u32 index = needed[i_needed];
            if (audio_stream_find_chunk(stream, index)) continue;

            // the slot of a chunk that isn't needed any more, there's always one since the ring is as big as the list
            audio_stream_chunk* slot = NULL;
            for (
                u32 i_chunk = 0;
                    i_chunk < AUDIO_STREAM_CHUNK_COUNT && !slot;
                  ++i_chunk) {

                audio_stream_chunk& chunk = stream->chunks[i_chunk];
                bool is_needed = false;
                for (u32 i = 0; i < needed_count; ++i) is_needed |= (needed[i] == chunk.index);
                if (!is_needed) slot = &chunk;
            }
            assert(slot);

            const u32 frame_start = index * AUDIO_STREAM_CHUNK_FRAMES;
            const u32 frames_left = frame_count - frame_start;
            const u32 frames      = (frames_left < AUDIO_STREAM_CHUNK_FRAMES) ? frames_left : AUDIO_STREAM_CHUNK_FRAMES;

            const bool did_read = asset_mngr_read_sound_samples(stream->info, frame_start, frames, slot->samples);
            slot->index       = did_read ? index  : INVALID_INDEX;
            slot->frame_count = did_read ? frames : 0;
            ++stream->chunk_reads;
        }
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

            // whatever is in the ring, a chunk that isn't there stops the read short
            case (audio_source_type_e_stream): {

                const audio_stream* stream   = src.stream;
                const u32           channels = src.channel_count;

                u32 done = 0;
                while (done < count) {

                    const u32                 frame_at = frame + done;
                    const u32                 index    = frame_at / AUDIO_STREAM_CHUNK_FRAMES;
                    const audio_stream_chunk* chunk    = audio_stream_find_chunk(stream, index);
                    if (!chunk) break;

                    const u32 offset = frame_at - (index * AUDIO_STREAM_CHUNK_FRAMES);
                    if (offset >= chunk->frame_count) break;

                    const u32  frames_left = chunk->frame_count - offset;
                    const u32  frames      = (count - done < frames_left) ? (count - done) : frames_left;
                    const s16* samples     = chunk->samples + (offset * channels);
                    f32*       dst         = out + (done * channels);
                    for (
                        u32 i_sample = 0;
                            i_sample < frames * channels;
                          ++i_sample) {

                        dst[i_sample] = (f32)samples[i_sample] / 32768.0f;
                    }
                    done += frames;
                }
                return(done);
            }
        }
        return(0);
    }

    // lets go of the sound asset, if the source has one pinned
    inline void
    audio_source_release(
        audio_source& src) {

        if (src.type == audio_source_type_e_memory && src.h_sound != INVALID_HANDLE) {
            asset_mngr_unpin(src.h_sound);
        }
        if (src.type == audio_source_type_e_stream) {
            src.stream->is_active = false;
        }
        src = {};
        src.h_sound = {INVALID_HANDLE};
    }
};
