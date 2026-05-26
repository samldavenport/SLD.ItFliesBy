#pragma once

#include "files.hpp"

namespace ifb {

    IFB_INTERNAL void
    file_close(
        file_manager*     mngr,
        const file_handle hnd) {

    }

    IFB_INTERNAL u32
    file_get_size(
        file_manager*     mngr,
        const file_handle hnd) {

    }

    IFB_INTERNAL void
    file_set_cursor(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         cursor) {

    }

    IFB_INTERNAL u32 
    file_read(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         buffer_size,
        cchar8*           buffer_ptr) {

    }

    IFB_INTERNAL u32 
    file_write(
        file_manager*     mngr,
        const file_handle hnd,
        const u32         buffer_size,
        const cchar8*     buffer_ptr) {
            
    }
};
