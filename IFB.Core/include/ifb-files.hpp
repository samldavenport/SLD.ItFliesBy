#ifndef IFB_FILES_HPP
#define IFB_FILES_HPP

#include "ifb.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "ifb-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    //TODO(SAM): this should be the same as INVALID_HANDLE
    constexpr u32 FILE_HANDLE_INVALID = 0;
    constexpr u32 FILE_INDEX_INVALID  = 0xFFFFFFFF;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES 
    //--------------------------------------------------------------------

    struct file_mngr;
    struct file_path;

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

   struct file_path {
        cchar cstr[IFB_CONFIG_FILE_PATH_SIZE];
    };

    struct file_mngr {
        struct {
            hnd_file*     handle_internal; 
            pfm_hnd_file* handle_platform;
            u32*             io_length;
            u32*             cursor;
            file_path*       paths;
        } array;
        reservation* res;
        byte*        file_data;
        u32          file_count_max;
        u32          file_granularity;
    };
};

#endif //IFB_FILES_HPP
