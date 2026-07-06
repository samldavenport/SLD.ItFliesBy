#pragma once

#include "files.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL file_mngr*
    file_mngr_create(
        void) {

        const auto& cfg = config_instance();
        assert(cfg.file_count != 0);

        // allocate memory
        auto mngr              = global_alloc<file_mngr>    ();
        auto hnd_file_internal = global_alloc<file_handle>     (cfg.file_count);
        auto hnd_file_platform = global_alloc<pfm_file_handle> (cfg.file_count);
        auto io_length         = global_alloc<u32>             (cfg.file_count);
        auto cursor            = global_alloc<u32>             (cfg.file_count);
        auto paths             = global_alloc<file_path>       (cfg.file_count);
        assert(
            mngr              != NULL &&
            hnd_file_internal != NULL &&
            hnd_file_platform != NULL &&
            io_length         != NULL &&
            cursor            != NULL &&
            paths             != NULL
        );

        _file_mngr                        = mngr;                        
        _file_mngr->array.handle_internal = hnd_file_internal; 
        _file_mngr->array.handle_platform = hnd_file_platform; 
        _file_mngr->array.io_length       = io_length; 
        _file_mngr->array.cursor          = cursor; 
        _file_mngr->array.paths           = paths; 
        _file_mngr->file_count_max        = cfg.file_count;

        return(_file_mngr);
    }

    IFB_INTERNAL void
    file_mngr_startup(
        const u32     mem_size,
        const u32     mem_granularity,
        void*         mem_ptr) {

        assert(
            _file_mngr      != NULL &&
            mem_size        != 0    &&
            mem_granularity != 0    &&
            mem_ptr         != NULL
        );

        _file_mngr->memory.start       = (byte*)mem_ptr;
        _file_mngr->memory.size        = mem_size;
        _file_mngr->memory.granularity = mem_granularity;
    }
    
    IFB_INTERNAL void
    file_mngr_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void
    file_mngr_assert_valid(
        void) {

        assert(
            _file_mngr                        != NULL                    &&
            _file_mngr->memory.start          != NULL                    &&
            _file_mngr->memory.size           != 0                       &&
            _file_mngr->memory.granularity    != 0                       &&
            _file_mngr->memory.granularity    <  _file_mngr->memory.size &&
            _file_mngr->array.handle_internal != NULL                    &&
            _file_mngr->array.handle_platform != NULL                    &&
            _file_mngr->array.io_length       != NULL                    &&
            _file_mngr->array.cursor          != NULL                    &&
            _file_mngr->array.paths           != NULL
        );
    }

    IFB_INTERNAL u32
    file_mngr_index_of_next_free(
        void) {

        file_mngr_assert_valid();

        u32 next_free_index = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_CONFIG_FILE_COUNT;
              ++index) {

            const bool is_free = (
                (_file_mngr->array.handle_internal[index] == FILE_HANDLE_INVALID) &&
                (_file_mngr->array.handle_platform[index] == FILE_HANDLE_INVALID)
            );

            if (is_free) {
                next_free_index = index;
                break;
            }
        }

        return(next_free_index);
    }

    IFB_INTERNAL u32
    file_mngr_index_of_internal_handle(
        const file_handle hnd) {

        file_mngr_assert_valid();

        u32 index_of = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_CONFIG_FILE_COUNT;
              ++index) {

            if (hnd == _file_mngr->array.handle_internal[index]) {
                index_of = index;
                break;
            }
        }

        return(index_of);
    }

    IFB_INTERNAL u32
    file_mngr_index_of_platform_handle(
        const pfm_file_handle hnd) {

        file_mngr_assert_valid();

        u32 index_of = FILE_INDEX_INVALID;

        for (
            u32 index = 0;
                index < IFB_CONFIG_FILE_COUNT;
              ++index) {

            if (hnd == _file_mngr->array.handle_platform[index]) {
                index_of = index;
                break;
            }
        }

        return(index_of);
    }

    IFB_INTERNAL byte*
    file_mngr_get_buffer(
        const u32 index) {

        file_mngr_assert_valid();
        assert(index < IFB_CONFIG_FILE_COUNT);

        const u32 offset = (index * _file_mngr->memory.granularity);
        byte* buffer     = &_file_mngr->memory.start[offset];

        return(buffer);
    }

    IFB_INTERNAL file_handle
    file_mngr_commit(
        const pfm_file_config* cfg) {

        file_mngr_assert_valid();
        assert(cfg);

        // get index of next free file
        const u32 index = file_mngr_index_of_next_free();
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file 
        const pfm_file_handle hnd_pfm = pfm_file_open(cfg);
        if (hnd_pfm == NULL) {
            return(FILE_HANDLE_INVALID);
        }

        // create the file handle from the path
        const u32         path_length = strnlen_s (cfg->path, IFB_CONFIG_FILE_PATH_SIZE);
        const file_handle hnd_ifb     = hash_u32  ((void*)&cfg->path[0], path_length);

        // commit memory
        const u32 offset = (index * _file_mngr->memory.granularity);
        void*     data   =  pfm_memory_commit(
            (void*)_file_mngr->memory.start,
            offset,
            _file_mngr->memory.granularity
        );
        assert(data);

        // update the table
        _file_mngr->array.handle_internal [index] = hnd_ifb;
        _file_mngr->array.handle_platform [index] = hnd_pfm;
        _file_mngr->array.io_length       [index] = 0;
        _file_mngr->array.cursor          [index] = 0;
        for (
            u32 c = 0;
            c < path_length;
            ++c
        ) {
            _file_mngr->array.paths[index].cstr[c] = cfg->path[c];
        }

        return(hnd_ifb);
    }

};