#pragma once

#include "eng-context.hpp"
#include "ifb-platform.hpp"
#include "file-internal.hpp"

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