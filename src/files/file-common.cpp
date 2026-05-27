#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL void
    file_close(
        file_manager*     mngr,
        const file_handle hnd) {

        assert(mngr != NULL && hnd != FILE_HANDLE_INVALID);

        const u32 index = file_manager_index_of_internal_handle(mngr, hnd);
        assert(index != FILE_INDEX_INVALID);


    }

    IFB_INTERNAL u32
    file_get_size(
        file_manager*     mngr,
        const file_handle hnd) {

        assert(mngr != NULL && hnd != FILE_HANDLE_INVALID);

        const u32 index = file_manager_index_of_internal_handle(mngr, hnd);

        const pfm_file_handle pfm_hnd = mngr->array.handle_platform[index];
        const u32             size    = pfm_file_size(pfm_hnd);
    
        return(size);
    }

    IFB_INTERNAL void
    file_set_cursor(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         cursor) {

        assert(mngr != NULL && hnd != FILE_HANDLE_INVALID);

        const u32 index = file_manager_index_of_internal_handle(mngr, hnd);

        mngr->array.cursor[index] = cursor;
    }

    IFB_INTERNAL const cchar8*
    file_read(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         buffer_size) {

        assert(
            mngr        != NULL                &&
            hnd         != FILE_HANDLE_INVALID &&
            buffer_size != 0
        );

        const u32 index  = file_manager_index_of_internal_handle (mngr, hnd);

        pfm_file_buffer file_buffer;
        file_buffer.data   = file_manager_get_buffer(mngr, index); 
        file_buffer.size   = mngr->memory.granularity; 
        file_buffer.length = 0;
        file_buffer.offset = 0;
        file_buffer.cursor = mngr->array.cursor[index];

        const pfm_file_handle hnd_pfm   = mngr->array.handle_platform[index];
        const u32             io_length = pfm_file_read(hnd_pfm, &file_buffer);

        mngr->array.io_length[index] = io_length;
        return((const cchar8*)file_buffer.data);
    }

    IFB_INTERNAL u32 
    file_write(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         buffer_size,
        byte*             buffer_ptr) {
      
        assert(
            mngr        != NULL                &&
            hnd         != FILE_HANDLE_INVALID &&
            buffer_size != 0                   &&
            buffer_ptr  != NULL
        );

        const u32 index  = file_manager_index_of_internal_handle (mngr, hnd);

        pfm_file_buffer file_buffer;
        file_buffer.data   = (byte*)buffer_ptr;
        file_buffer.size   = buffer_size;
        file_buffer.length = 0;
        file_buffer.offset = 0;
        file_buffer.cursor = mngr->array.cursor[index];

        const pfm_file_handle hnd_pfm   = mngr->array.handle_platform[index];
        const u32             io_length = pfm_file_read(hnd_pfm, &file_buffer);

        mngr->array.io_length[index] = io_length;
        return(io_length);

    }
};
