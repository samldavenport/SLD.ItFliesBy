#pragma once

#include "ifb.hpp"
#include "eng-wav.hpp"
#include "eng-asset.hpp"
#include "eng-jobs.hpp"
#include "ifb-platform.hpp"

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

    // a chunk is read by a job on a worker thread, only the state crosses between threads, under the stream's lock
    enum audio_stream_chunk_state_e {
        audio_stream_chunk_state_e_empty   = 0,
        audio_stream_chunk_state_e_loading = 1, // a job is writing the samples, nothing else touches the chunk until it's done
        audio_stream_chunk_state_e_ready   = 2,
        audio_stream_chunk_state_e_failed  = 3
    };

    // a sound is split into chunks of AUDIO_STREAM_CHUNK_FRAMES, and the last one is shorter
    // the samples are as they are in the bank, 16 bit with the sound's channels
    // the main thread decides which chunk a slot holds, and only reads the samples once it has seen the job finish
    struct audio_stream_chunk {
        u32  index;       // which chunk of the sound, INVALID_INDEX if it's empty, main thread only
        u32  frame_count; // main thread only
        u32  state;       // audio_stream_chunk_state_e, under the stream's lock
        bool is_ready;    // the main thread has seen it finish, so the mixer can read it, main thread only
        s16* samples;     // room for AUDIO_STREAM_CHUNK_FRAMES of AUDIO_STREAM_CHANNELS_MAX
    };

    // a ring of chunks, refilled before the voice is mixed so it holds the chunk under the playhead and the ones after it
    // nothing is pinned, the samples come from the bank as they're needed
    // a stream with a chunk still loading can't be opened again, even once its voice is gone, so a late job can't write into another sound
    struct audio_stream {
        asset_sound_stream_info info;
        hnd_asset               h_sound;
        pfm_thread_mutex        lock;         // covers the chunks' state
        u32                     chunk_reads;  // every chunk read from the bank, for the stats
        bool                    is_active;
        audio_stream_chunk      chunks[AUDIO_STREAM_CHUNK_COUNT];
    };

    // what a read job needs, the stream's info doesn't change while one of its chunks is loading
    struct audio_stream_read_ctx {
        audio_stream* stream;
        u32           chunk;
        u32           frame_start;
        u32           frame_count;
    };
    static_assert(sizeof(audio_stream_read_ctx) <= sizeof(job_ctx), "audio_stream_read_ctx doesn't fit in a job");

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

    // false if a job is still writing one of its chunks
    inline bool
    audio_stream_is_loading(
        audio_stream* stream) {

        bool is_loading = false;
        pfm_thread_mutex_lock(stream->lock);
        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            is_loading |= (stream->chunks[i_chunk].state == audio_stream_chunk_state_e_loading);
        }
        pfm_thread_mutex_unlock(stream->lock);
        return(is_loading);
    }

    // the chunks start out empty, so the first refill asks for them
    // nothing can be loading, see audio_stream_is_loading
    inline void
    audio_stream_open(
        audio_stream*                  stream,
        const hnd_asset                h_sound,
        const asset_sound_stream_info& info) {

        assert(stream && !stream->is_active);
        assert(info.channel_count >= 1 && info.channel_count <= AUDIO_STREAM_CHANNELS_MAX && info.frame_count != 0);
        assert(!audio_stream_is_loading(stream));

        stream->info      = info;
        stream->h_sound   = h_sound;
        stream->is_active = true;
        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            stream->chunks[i_chunk].index       = INVALID_INDEX;
            stream->chunks[i_chunk].frame_count = 0;
            stream->chunks[i_chunk].state       = audio_stream_chunk_state_e_empty;
            stream->chunks[i_chunk].is_ready    = false;
        }
    }

    // the chunk with that index, if the main thread has seen it finish
    inline const audio_stream_chunk*
    audio_stream_find_chunk(
        const audio_stream* stream,
        const u32           index) {

        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            const audio_stream_chunk& chunk = stream->chunks[i_chunk];
            if (chunk.index == index && chunk.is_ready) return(&chunk);
        }
        return(NULL);
    }

    // runs on a worker thread, it only writes the chunk's samples and then its state
    IFB_INTERNAL void
    audio_stream_read_job(
        const hnd_arena h_arena,
        void*           ctx) {

        (void)h_arena;
        const audio_stream_read_ctx* read  = (const audio_stream_read_ctx*)ctx;
        audio_stream*                stream = read->stream;
        audio_stream_chunk&          chunk  = stream->chunks[read->chunk];

        const bool did_read = asset_mngr_read_sound_samples(stream->info, read->frame_start, read->frame_count, chunk.samples);

        pfm_thread_mutex_lock(stream->lock);
        chunk.state = did_read ? audio_stream_chunk_state_e_ready : audio_stream_chunk_state_e_failed;
        pfm_thread_mutex_unlock(stream->lock);
    }

    // marks the reads that finished as ready for the mixer, and empties the ones that failed so they're asked for again
    // the stream's lock has to be held
    inline void
    audio_stream_pick_up_reads(
        audio_stream* stream) {

        for (
            u32 i_chunk = 0;
                i_chunk < AUDIO_STREAM_CHUNK_COUNT;
              ++i_chunk) {

            audio_stream_chunk& chunk = stream->chunks[i_chunk];
            if (chunk.state == audio_stream_chunk_state_e_ready) {
                chunk.is_ready = true;
            }
            else if (chunk.state == audio_stream_chunk_state_e_failed) {
                chunk.index    = INVALID_INDEX;
                chunk.is_ready = false;
                chunk.state    = audio_stream_chunk_state_e_empty;
            }
        }
    }

    // makes sure the ring has, or is reading, the chunk the position is in and the ones after it
    // a looping stream's chunks after the last one are the first ones again, so a loop doesn't wait on the disk
    // the reads are jobs, so a chunk asked for now is ready a frame or more later
    // a chunk that fails to read is asked for again, and a slot that's still loading isn't reused until it's done
    // returns the chunks it asked for
    inline u32
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

        audio_stream_read_ctx reads[AUDIO_STREAM_CHUNK_COUNT];
        u32                   read_count = 0;

        pfm_thread_mutex_lock(stream->lock);
        audio_stream_pick_up_reads(stream);

        for (
            u32 i_needed = 0;
                i_needed < needed_count;
              ++i_needed) {

            const u32 index = needed[i_needed];

            // already there, or on its way
            bool is_present = false;
            for (
                u32 i_chunk = 0;
                    i_chunk < AUDIO_STREAM_CHUNK_COUNT;
                  ++i_chunk) {

                const audio_stream_chunk& chunk = stream->chunks[i_chunk];
                is_present |= (chunk.index == index && chunk.state != audio_stream_chunk_state_e_empty);
            }
            if (is_present) continue;

            // the slot of a chunk that isn't needed any more, and isn't being written
            // after a seek they can all be busy for a moment, then it's asked for on a later refill
            u32 slot = INVALID_INDEX;
            for (
                u32 i_chunk = 0;
                    i_chunk < AUDIO_STREAM_CHUNK_COUNT && slot == INVALID_INDEX;
                  ++i_chunk) {

                const audio_stream_chunk& chunk = stream->chunks[i_chunk];
                if (chunk.state == audio_stream_chunk_state_e_loading) continue;

                bool is_needed = false;
                for (u32 i = 0; i < needed_count; ++i) is_needed |= (needed[i] == chunk.index);
                if (!is_needed) slot = i_chunk;
            }
            if (slot == INVALID_INDEX) break;

            const u32 frame_start = index * AUDIO_STREAM_CHUNK_FRAMES;
            const u32 frames_left = frame_count - frame_start;
            const u32 frames      = (frames_left < AUDIO_STREAM_CHUNK_FRAMES) ? frames_left : AUDIO_STREAM_CHUNK_FRAMES;

            audio_stream_chunk& chunk = stream->chunks[slot];
            chunk.index       = index;
            chunk.frame_count = frames;
            chunk.is_ready    = false;
            chunk.state       = audio_stream_chunk_state_e_loading;

            reads[read_count++] = { stream, slot, frame_start, frames };
        }

        pfm_thread_mutex_unlock(stream->lock);

        // the jobs go out after the lock, with one core they run before job_dspchr_add_job returns, and take the lock themselves
        for (
            u32 i_read = 0;
                i_read < read_count;
              ++i_read) {

            job j;
            j.proc = audio_stream_read_job;
            memcpy(j.ctx.data, &reads[i_read], sizeof(audio_stream_read_ctx));

            // a full queue gives the slot back, it's asked for again on the next refill
            if (!job_dspchr_add_job(j)) {
                pfm_thread_mutex_lock(stream->lock);
                audio_stream_chunk& chunk = stream->chunks[reads[i_read].chunk];
                chunk.index = INVALID_INDEX;
                chunk.state = audio_stream_chunk_state_e_empty;
                pfm_thread_mutex_unlock(stream->lock);
            }
            ++stream->chunk_reads;
        }

        // a read can already be done, like with one core where the job runs before it's added, so it's usable this frame
        if (read_count != 0) {
            pfm_thread_mutex_lock(stream->lock);
            audio_stream_pick_up_reads(stream);
            pfm_thread_mutex_unlock(stream->lock);
        }
        return(read_count);
    }

    // true if the chunks for count frames from position on are ready, wrapping a looping stream and stopping at the end of one that isn't
    inline bool
    audio_stream_has_frames(
        const audio_stream* stream,
        const f64           position,
        const u32           count,
        const bool          is_looping) {

        const u32 frame_count = stream->info.frame_count;
        u32       frame       = (u32)position;
        if (frame >= frame_count) {
            if (!is_looping) return(true);  // the voice is about to finish, it doesn't read anything
            frame %= frame_count;
        }

        // every chunk the frames touch, a short last chunk and a wrap can make that more than 2
        u32 remaining = count;
        while (remaining != 0) {

            const u32 index = frame / AUDIO_STREAM_CHUNK_FRAMES;
            if (!audio_stream_find_chunk(stream, index)) return(false);

            const u32 chunk_end = ((index + 1) * AUDIO_STREAM_CHUNK_FRAMES < frame_count) ? ((index + 1) * AUDIO_STREAM_CHUNK_FRAMES) : frame_count;
            const u32 frames    = (remaining < chunk_end - frame) ? remaining : (chunk_end - frame);
            remaining -= frames;
            frame     += frames;

            // the end of the sound, a looping one comes back around and one that isn't only holds its last frame
            if (frame >= frame_count) {
                if (!is_looping) break;
                frame = 0;
            }
        }
        return(true);
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
