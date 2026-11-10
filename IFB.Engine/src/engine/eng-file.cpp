#pragma once

#include "eng-context.hpp"
#include "ifb-platform.hpp"
#include "eng-file.hpp"

namespace ifb {

    IFB_ENGINE_API hnd_file
    eng_file_ro_create_new(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_ro_create_new(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_ro_open_existing(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_ro_open_existing(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_ro_open_always(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_ro_open_always(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_ro_overwrite(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_ro_overwrite(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_wo_create_new(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_wo_create_new(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_wo_open_existing(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_wo_open_existing(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_wo_open_always(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_wo_open_always(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_wo_overwrite(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_wo_overwrite(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_rw_create_new(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_rw_create_new(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_rw_open_existing(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_rw_open_existing(path);
        return(hnd);

    }

    IFB_ENGINE_API hnd_file
    eng_file_rw_open_always(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_rw_open_always(path);
        return(hnd);
    }

    IFB_ENGINE_API hnd_file
    eng_file_rw_overwrite(
        const cchar* path) {

        assert(path);
        hnd_file hnd = file_rw_overwrite(path);
        return(hnd);
    }

    IFB_ENGINE_API void
    eng_file_close(
        const hnd_file hnd) {

        file_close(hnd.val);
    }

    IFB_ENGINE_API u32
    eng_file_get_size(
        const hnd_file hnd) {

        const u32 size = file_get_size(hnd.val);
        return(size);
    }

    IFB_ENGINE_API void
    eng_file_set_cursor(
        const hnd_file hnd,
        const u32             cursor) {

        file_set_cursor(hnd.val, cursor);
    }

    IFB_ENGINE_API const cchar*
    eng_file_read(
        const hnd_file hnd) {

        const u32     buffer_size = file_get_size(hnd.val);
        const cchar* buffer_ptr  = file_read(
            hnd.val,
            buffer_size
        );

        return(buffer_ptr);
    }

    // reads up to buffer_size bytes from the cursor into the caller's buffer
    // returns the number of bytes read
    IFB_ENGINE_API u32
    eng_file_read_to_buffer(
        const hnd_file hnd,
        const u32      buffer_size,
        byte*          buffer_ptr) {

        assert(buffer_size != 0 && buffer_ptr != NULL);

        const u32 bytes_read = file_read_to_buffer(hnd.val, buffer_size, buffer_ptr);
        return(bytes_read);
    }

    // reads the whole file into a new block on the arena, starting from the
    // beginning of the file (this sets the cursor to 0)
    // returns an empty memory block if the file is empty, the arena is full, or the read fails
    IFB_ENGINE_API memory
    eng_file_read_to_arena(
        const hnd_file  hnd,
        const hnd_arena arena) {

        memory mem;
        mem.size = 0;
        mem.ptr  = NULL;

        const u32 file_size = file_get_size(hnd.val);
        if (file_size == 0) {
            return(mem);
        }

        const u32 save       = arena_save(arena);
        byte*     buffer     = (byte*)arena_push(arena, file_size);
        if (!buffer) {
            return(mem);
        }

        file_set_cursor(hnd.val, 0);
        const u32 bytes_read = file_read_to_buffer(hnd.val, file_size, buffer);
        if (bytes_read != file_size) {
            arena_revert(arena, save);
            return(mem);
        }

        mem.size  = file_size;
        mem.bytes = buffer;
        return(mem);
    }

    IFB_ENGINE_API u32
    eng_file_write(
        const hnd_file hnd,
        const u32             buffer_size,
        const byte*           buffer_ptr) {

        assert(buffer_size != 0 && buffer_ptr != NULL);

        const u32 bytes_written = file_write(hnd.val, buffer_size, buffer_ptr);
        return(bytes_written);
    }

    IFB_ENGINE_API const cchar*
    eng_file_dialog_open(
        const cchar* filter_type,
        const cchar* filter_desc) {

        pfm_file_dialog dlg;
        dlg.path        = NULL;
        dlg.filter_type = filter_type;
        dlg.filter_desc = filter_desc;

        const bool did_select = pfm_file_dialog_open(dlg);
        return(did_select ? dlg.path : NULL);
    }

    IFB_ENGINE_API const cchar*
    eng_file_dialog_save(
        const cchar* filter_type,
        const cchar* filter_desc) {

        pfm_file_dialog dlg;
        dlg.path        = NULL;
        dlg.filter_type = filter_type;
        dlg.filter_desc = filter_desc;

        const bool did_select = pfm_file_dialog_save(dlg);
        return(did_select ? dlg.path : NULL);
    }
};