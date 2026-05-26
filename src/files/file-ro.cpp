#pragma once

#include "files.hpp"

namespace ifb {


    IFB_INTERNAL file_handle
    file_ro_create_new(
        file_manager* mngr,
        const cchar8* path) {

        file_manager_assert_valid(mngr);
        assert(path);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free(mngr);
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_create_new;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_file_handle pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // create the file handle from the path
        const u32         path_length = strnlen_s (path, IFB_FILE_PATH_SIZE);
        const file_handle our_hnd     = hash_u32  (path, path_length);

        // update the table
        file_manager_commit(mngr, index, our_hnd, pfm_hnd);

        // return the created handle
        return(our_hnd);
    }

    IFB_INTERNAL file_handle
    file_ro_open_existing(
        file_manager* mngr,
        const cchar8* path) {

        file_manager_assert_valid(mngr);
        assert(path);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free(mngr);
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_existing;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_file_handle pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // create the file handle from the path
        const u32         path_length = strnlen_s (path, IFB_FILE_PATH_SIZE);
        const file_handle our_hnd     = hash_u32  (path, path_length);

        // update the table
        file_manager_commit(mngr, index, our_hnd, pfm_hnd);

        // return the created handle
        return(our_hnd);
    }

    IFB_INTERNAL file_handle
    file_ro_open_always(
        file_manager* mngr,
        const cchar8* path) {

        file_manager_assert_valid(mngr);
        assert(path);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free(mngr);
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_always;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_file_handle pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // create the file handle from the path
        const u32         path_length = strnlen_s (path, IFB_FILE_PATH_SIZE);
        const file_handle our_hnd     = hash_u32  (path, path_length);

        // update the table
        file_manager_commit(mngr, index, our_hnd, pfm_hnd);

        // return the created handle
        return(our_hnd);
    }

    IFB_INTERNAL file_handle
    file_ro_overwrite(
        file_manager* mngr,
        const cchar8* path) {

        file_manager_assert_valid(mngr);
        assert(path);

        // get index of next free file
        const u32 index = file_manager_index_of_next_free(mngr);
        if (index == FILE_INDEX_INVALID) {
            return(FILE_HANDLE_INVALID);
        }

        // open the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_overwrite_existing;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_file_handle pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // create the file handle from the path
        const u32         path_length = strnlen_s (path, IFB_FILE_PATH_SIZE);
        const file_handle our_hnd     = hash_u32  (path, path_length);

        // update the table
        file_manager_commit(mngr, index, our_hnd, pfm_hnd);

        // return the created handle
        return(our_hnd);
    }

};