#pragma once

#include "files.hpp"

namespace ifb {


    IFB_INTERNAL hnd_file
    file_ro_create_new(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_create_new;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;

        // return the created handle
        const hnd_file hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL hnd_file
    file_ro_open_existing(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_existing;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_hnd_file pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }

        // return the created handle
        const hnd_file hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL hnd_file
    file_ro_open_always(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_always;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_hnd_file pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // return the created handle
        const hnd_file hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL hnd_file
    file_ro_overwrite(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_overwrite_existing;
        file_config.access_flags = pfm_file_access_flag_e_read;
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;
        const pfm_hnd_file pfm_hnd = pfm_file_open(&file_config);
        if (pfm_hnd == NULL) {
            return(FILE_HANDLE_INVALID);
        }
        
        // return the created handle
        const hnd_file hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

};