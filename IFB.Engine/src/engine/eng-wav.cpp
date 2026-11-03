#pragma once

#include "ifb-engine.hpp"
#include "eng-file.hpp"
#include "eng-memory.hpp"
#include "eng-wav.hpp"

namespace ifb {

    IFB_ENGINE_API const wav*
    eng_wav_load_to_arena(
        const cchar*    path,
        const hnd_arena h_arena) {

        assert(path);

        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) return(NULL);

        // the file is only needed for parsing, so it gets its own arena
        // that leaves the whole of h_arena for the samples
        const wav* w         = NULL;
        const u32  size_file = file_get_size(h_file);
        const hnd_arena h_arena_file = (size_file != 0) ? arena_large_alloc() : hnd_arena{INVALID_HANDLE};

        if (h_arena_file != INVALID_HANDLE) {

            byte* file_data = (byte*)arena_push(h_arena_file, size_file);
            if (file_data) {
                file_set_cursor(h_file, 0);
                const u32 bytes_read = file_read_to_buffer(h_file, size_file, file_data);
                if (bytes_read == size_file) {
                    w = wav_parse_data(file_data, size_file, h_arena);
                }
            }
            arena_free(h_arena_file);
        }

        file_close(h_file);
        return(w);
    }

    IFB_ENGINE_API u32
    eng_wav_get_frame_count(
        const wav* w) {

        assert(w);
        return(w->data.size / w->format.block_align);
    }

    IFB_ENGINE_API f32
    eng_wav_get_sample(
        const wav* w,
        const u32  frame,
        const u32  channel) {

        return(wav_read_sample(w, frame, channel));
    }
};
