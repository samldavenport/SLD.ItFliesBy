#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL void
    file_close(
        const file_handle hnd) {

        file_mngr_assert_valid();
        assert(hnd != FILE_HANDLE_INVALID);

        const u32 index = file_mngr_index_of_internal_handle(hnd);
        assert(index != FILE_INDEX_INVALID);
    }

    IFB_INTERNAL u32
    file_get_size(
        const file_handle hnd) {

        file_mngr_assert_valid();
        assert(hnd != FILE_HANDLE_INVALID);

        const u32 index = file_mngr_index_of_internal_handle(hnd);

        const pfm_file_handle pfm_hnd = _file_mngr->array.handle_platform[index];
        const u32             size    = pfm_file_size(pfm_hnd);
    
        return(size);
    }

    IFB_INTERNAL void
    file_set_cursor(
        const file_handle hnd,
        const u32         cursor) {

        file_mngr_assert_valid();
        assert(hnd != FILE_HANDLE_INVALID);

        const u32 index = file_mngr_index_of_internal_handle(hnd);

        _file_mngr->array.cursor[index] = cursor;
    }

    IFB_INTERNAL const cchar*
    file_read(
        const file_handle hnd,
        const u32         buffer_size) {

        file_mngr_assert_valid();
        assert(
            hnd         != FILE_HANDLE_INVALID &&
            buffer_size != 0
        );

        const u32 index  = file_mngr_index_of_internal_handle (hnd);

        pfm_file_buffer file_buffer;
        file_buffer.data   = file_mngr_get_buffer(index); 
        file_buffer.size   = _file_mngr->memory.granularity; 
        file_buffer.length = 0;
        file_buffer.offset = 0;
        file_buffer.cursor = _file_mngr->array.cursor[index];

        const pfm_file_handle hnd_pfm   = _file_mngr->array.handle_platform[index];
        const u32             io_length = pfm_file_read(hnd_pfm, &file_buffer);

        _file_mngr->array.io_length[index] = io_length;

        const u32 null_index = (io_length < file_buffer.size)
            ? io_length
            : file_buffer.size - 1;

        if (file_buffer.data[null_index] != 0) {
            file_buffer.data[null_index] = 0;
        }

        return((const cchar*)file_buffer.data);
    }

    IFB_INTERNAL u32 
    file_write(
        const file_handle hnd,
        const u32         buffer_size,
        const byte*       buffer_ptr) {

        file_mngr_assert_valid();
        assert(
            hnd         != FILE_HANDLE_INVALID &&
            buffer_size != 0                   &&
            buffer_ptr  != NULL
        );

        const u32 index  = file_mngr_index_of_internal_handle (hnd);

        pfm_file_buffer file_buffer;
        file_buffer.data   = (byte*)buffer_ptr;
        file_buffer.size   = buffer_size;
        file_buffer.length = 0;
        file_buffer.offset = 0;
        file_buffer.cursor = _file_mngr->array.cursor[index];

        const pfm_file_handle hnd_pfm   = _file_mngr->array.handle_platform[index];
        const u32             io_length = pfm_file_read(hnd_pfm, &file_buffer);

        _file_mngr->array.io_length[index] = io_length;
        return(io_length);
    }
};
