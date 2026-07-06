#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL file_handle
    file_wo_create_new(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path != NULL);

        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_create_new;
        file_config.access_flags = pfm_file_access_flag_e_write;
        file_config.share_flags  = pfm_file_share_flag_e_none;
        file_config.is_async     = false;
        
        const file_handle hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_wo_open_existing(
        const cchar* path) {
        
        file_mngr_assert_valid();
        assert(path != NULL);

        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_existing;
        file_config.access_flags = pfm_file_access_flag_e_write;
        file_config.share_flags  = pfm_file_share_flag_e_none;
        file_config.is_async     = false;
        
        const file_handle hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_wo_open_always(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path != NULL);

        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_always;
        file_config.access_flags = pfm_file_access_flag_e_write;
        file_config.share_flags  = pfm_file_share_flag_e_none;
        file_config.is_async     = false;
        
        const file_handle hnd = file_mngr_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_wo_overwrite(
        const cchar* path) {

        file_mngr_assert_valid();
        assert(path != NULL);

        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_overwrite_existing;
        file_config.access_flags = pfm_file_access_flag_e_write;
        file_config.share_flags  = pfm_file_share_flag_e_none;
        file_config.is_async     = false;
        
        const file_handle hnd = file_mngr_commit(&file_config);
        return(hnd);
    }
};