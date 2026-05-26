#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL void
    file_manager_startup(
        file_manager* mngr,
        const u32     mem_size,
        const u32     mem_granularity,
        void*         mem_ptr) {

        assert(
            mngr            != NULL &&
            mem_size        != 0    &&
            mem_granularity != 0    &&
            mem_ptr         != NULL
        );

        mngr->memory.start       = (byte*)mem_ptr;
        mngr->memory.size        = mem_size;
        mngr->memory.granularity = mem_granularity;

        for (
            u32 index = 0;
                index < IFB_FILE_COUNT;
              ++index) {

            mngr->array.handles[index] = 0;
        }
    }
    
    IFB_INTERNAL void
    file_manager_shutdown(
        file_manager* mngr) {

        //TODO
    }

    IFB_INTERNAL void
    file_manager_assert_valid(
        const file_manager* mngr) {

        assert(
            mngr                     != NULL &&
            mngr->memory.start       != NULL &&
            mngr->memory.size        != 0    &&
            mngr->memory.granularity != 0    &&
            mngr->memory.granularity <  mngr->memory.size
        );
    }

    IFB_INTERNAL u32
    file_manager_index_of_next_free(
        const file_manager* mngr) {

        file_manager_assert_valid(mngr);

        u32 next_free_index = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_FILE_COUNT;
              ++index) {

            const bool is_free = (
                (mngr->array.handle_internal[index] == FILE_HANDLE_INVALID) &&
                (mngr->array.handle_platform[index] == FILE_HANDLE_INVALID)
            );

            if (is_free) {
                next_free_index = index;
                break;
            }
        }

        return(next_free_index);
    }

    IFB_INTERNAL u32
    file_manager_index_of_internal_handle(
        const file_manager*   mngr,
        const pfm_file_handle hnd) {

        file_manager_assert_valid(mngr);

        u32 index_of = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_FILE_COUNT;
              ++index) {

            if (hnd == mngr->array.handle_internal[index]) {
                index_of = index;
                break;
            }
        }

        return(index_of);
    }

    IFB_INTERNAL u32
    file_manager_index_of_platform_handle(
        const file_manager*   mngr,
        const pfm_file_handle hnd) {

        file_manager_assert_valid(mngr);

        u32 index_of = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_FILE_COUNT;
              ++index) {

            if (hnd == mngr->array.handle_platform[index]) {
                index_of = index;
                break;
            }
        }

        return(index_of);
    }

    IFB_INTERNAL byte*
    file_manager_get_buffer(
        const file_manager* mngr,
        const u32           index) {

        file_manager_assert_valid(mngr);
        assert(index < IFB_FILE_COUNT);

        const u32 offset = (index * mngr->memory.granularity);
        byte* buffer     = &mngr->memory.start[offset];

        return(buffer);
    }

    IFB_INTERNAL void
    file_manager_commit(
        const file_manager*   mngr,
        const u32             file_index,
        const file_handle     hnd_ifb,
        const pfm_file_handle hnd_pfm,
        const cchar8*         path) {

        file_manager_assert_valid(mngr);

        assert(
            file_index <  IFB_FILE_COUNT      &&
            hnd_ifb    != FILE_HANDLE_INVALID &&
            hnd_pfm    != NULL                &&
            path       != NULL
        );

  
    }

    IFB_INTERNAL file_handle
    file_manager_commit(
        file_manager*          mngr,
        const pfm_file_config* cfg) {

        file_manager_assert_valid(mngr);
        assert(cfg);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free(mngr);
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file 
        const pfm_file_handle hnd_pfm = pfm_file_open(&file_config);
        if (hnd_pfm == NULL) {
            return(FILE_HANDLE_INVALID);
        }

        // create the file handle from the path
        const u32         path_length = strnlen_s (path, IFB_FILE_PATH_SIZE);
        const file_handle hnd_ifb     = hash_u32  (path, path_length);

        // commit memory
        const u32 offset = (index * mngr->memory.granularity);
        void*     data   =  pfm_memory_commit(
            (void*)mngr->memory.start,
            offset,
            mngr->memory.granularity
        );
        assert(data);

        // update the table
        mngr->array.handle_internal [file_index] = hnd_ifb;
        mngr->array.handle_platform [file_index] = hnd_pfm;
        mngr->array.io_length       [file_index] = 0;
        mngr->array.cursor          [file_index] = 0;

        const u32 path_length = strnlen_s(path, IFB_FILE_PATH_SIZE);
        strncpy_s(
            (cchar8*)&mngr->array.paths[file_index].cstr[0],
            path_length,
            path,
            IFB_FILE_PATH_SIZE            
        );
    }

};