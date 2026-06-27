#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL u32
    file_manager_memory_requirement(
        const u32 file_count_max) {

        assert(file_count_max != 0);

        const u32 struct_size    = sizeof(file_manager);
        const u32 file_info_size = (
            (file_count_max * sizeof(file_handle))     +
            (file_count_max * sizeof(pfm_file_handle)) +
            (file_count_max * sizeof(u32))             +
            (file_count_max * sizeof(u32))             +
            (file_count_max * sizeof(file_path))
        );

        const u32 mem_req = (struct_size + file_info_size);
        return(mem_req);
    }

    IFB_INTERNAL file_manager*
    file_manager_init(
        const u32 file_count_max,
        const u32 mem_size,
        void*     mem_ptr) {

        assert(
            file_count_max != 0 &&
            mem_size       != 0 &&
            mem_ptr        != NULL          
        );

        const u32 mem_req = file_manager_memory_requirement(file_count_max);
        assert(mem_req == mem_size);

        const u32 offset_hnd_ifb   = sizeof(file_manager); 
        const u32 offset_hnd_pfm   = offset_hnd_ifb + (file_count_max + sizeof(file_handle)); 
        const u32 offset_io        = offset_hnd_pfm + (file_count_max + sizeof(pfm_file_handle)); 
        const u32 offset_cursor    = offset_io      + (file_count_max + sizeof(u32)); 
        const u32 offset_file_path = offset_cursor  + (file_count_max + sizeof(u32)); 

        byte* mem_handle_internal = ((byte*)mem_ptr             + offset_hnd_ifb);
        byte* mem_handle_platform = ((byte*)mem_handle_internal + offset_hnd_pfm);
        byte* mem_io_length       = ((byte*)mem_handle_platform + offset_io);
        byte* mem_cursor          = ((byte*)mem_io_length       + offset_cursor);
        byte* mem_paths           = ((byte*)mem_cursor          + offset_file_path);

        _file_mngr = (file_manager*)mem_ptr;
        _file_mngr->array.handle_internal =     (file_handle*)mem_handle_internal;
        _file_mngr->array.handle_platform = (pfm_file_handle*)mem_handle_platform;
        _file_mngr->array.io_length       =             (u32*)mem_io_length;
        _file_mngr->array.cursor          =             (u32*)mem_cursor;
        _file_mngr->array.paths           =       (file_path*)mem_paths;
        _file_mngr->file_count_max        = file_count_max;
    
        assert(
            _file_mngr->array.handle_internal != NULL &&
            _file_mngr->array.handle_platform != NULL &&
            _file_mngr->array.io_length       != NULL &&
            _file_mngr->array.cursor          != NULL &&
            _file_mngr->array.paths           != NULL            
        );

        return(_file_mngr);
    }

    IFB_INTERNAL void
    file_manager_startup(
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
    file_manager_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void
    file_manager_assert_valid(
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
    file_manager_index_of_next_free(
        void) {

        file_manager_assert_valid();

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
    file_manager_index_of_internal_handle(
        const file_handle hnd) {

        file_manager_assert_valid();

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
    file_manager_index_of_platform_handle(
        const pfm_file_handle hnd) {

        file_manager_assert_valid();

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
    file_manager_get_buffer(
        const u32 index) {

        file_manager_assert_valid();
        assert(index < IFB_CONFIG_FILE_COUNT);

        const u32 offset = (index * _file_mngr->memory.granularity);
        byte* buffer     = &_file_mngr->memory.start[offset];

        return(buffer);
    }

    IFB_INTERNAL file_handle
    file_manager_commit(
        const pfm_file_config* cfg) {

        file_manager_assert_valid();
        assert(cfg);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free();
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