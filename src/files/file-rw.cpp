#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL file_handle
    file_rw_create_new(
        const cchar8* path) {

        file_manager_assert_valid();
        assert(path != NULL);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_create_new;
        file_config.access_flags = (pfm_file_access_flag_e_read | pfm_file_access_flag_e_write);
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;

        // return the created handle
        const file_handle hnd = file_manager_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_rw_open_existing(
        file_manager* mngr,
        const cchar8* path) {

        file_manager_assert_valid();
        assert(path != NULL);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_existing;
        file_config.access_flags = (pfm_file_access_flag_e_read | pfm_file_access_flag_e_write);
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;

        // return the created handle
        const file_handle hnd = file_manager_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_rw_open_always(
        const cchar8* path) {

        file_manager_assert_valid();
        assert(path != NULL);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_open_always;
        file_config.access_flags = (pfm_file_access_flag_e_read | pfm_file_access_flag_e_write);
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;

        // return the created handle
        const file_handle hnd = file_manager_commit(&file_config);
        return(hnd);
    }

    IFB_INTERNAL file_handle
    file_rw_overwrite(
        const cchar8* path) {

        file_manager_assert_valid();
        assert(path != NULL);

        // configure the file
        pfm_file_config file_config;
        file_config.path         = path;
        file_config.mode         = pfm_file_mode_e_overwrite_existing;
        file_config.access_flags = (pfm_file_access_flag_e_read | pfm_file_access_flag_e_write);
        file_config.share_flags  = pfm_file_share_flag_e_read;
        file_config.is_async     = false;

        // return the created handle
        const file_handle hnd = file_manager_commit(&file_config);
        return(hnd);
    }
};