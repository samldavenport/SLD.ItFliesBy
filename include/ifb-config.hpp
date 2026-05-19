#ifndef IFB_CONFIG_HPP
#define IFB_CONFIG_HPP

#include "sld.hpp"

using namespace sld;

#define IFB_CONFIG_BUILD_MODE           0
#define IFB_CONFIG_WINDOW_TITLE_DEBUG   "It Flies By (Debug)"
#define IFB_CONFIG_WINDOW_TITLE_RELEASE "It Flies By"
#define IFB_CONFIG_WINDOW_TITLE_SIZE    32
#define IFB_CONFIG_WINDOW_START_WIDTH   1024
#define IFB_CONFIG_WINDOW_START_HEIGHT  768
#define IFB_CONFIG_WINDOW_MAX_KEYS      8
#define IFB_CONFIG_DEFAULT_FPS          60
#define IFB_MEMORY_RES_SIZE_CORE        size_megabytes(64);
#define IFB_MEMORY_RES_SIZE_FILES       size_megabytes(64);
#define IFB_MEMORY_RES_SIZE_RENDERING   size_megabytes(64);

#if (IFB_CONFIG_BUILD_MODE==0)
#   define IFB_CONFIG_WINDOW_TITLE IFB_CONFIG_WINDOW_TITLE_DEBUG
#else
#   define  IFB_CONFIG_WINDOW_TITLE IFB_CONFIG_WINDOW_TITLE_RELEASE
#endif

namespace ifb {

    enum config_mode_e {
        config_mode_e_debug   = 0,
        config_mode_e_release = 1
    };

    struct config {
        static constexpr u32 build_mode            = IFB_CONFIG_BUILD_MODE;
        static constexpr u32 window_start_width    = IFB_CONFIG_WINDOW_START_WIDTH;
        static constexpr u32 window_start_height   = IFB_CONFIG_WINDOW_START_HEIGHT;
        static constexpr u32 window_max_keys       = IFB_CONFIG_WINDOW_MAX_KEYS;
        static constexpr u32 default_fps           = IFB_CONFIG_DEFAULT_FPS;
        static constexpr u32 memory_size_core      = IFB_MEMORY_RES_SIZE_CORE;
        static constexpr u32 memory_size_files     = IFB_MEMORY_RES_SIZE_FILES;
        static constexpr u32 memory_size_rendering = IFB_MEMORY_RES_SIZE_RENDERING;
        
        static constexpr cchar8 window_title[IFB_CONFIG_WINDOW_TITLE_SIZE] = {
            IFB_CONFIG_WINDOW_TITLE
        }; 
    };

    inline config&
    config_instance(void) {
        static config cfg;
        return(cfg);
    }
};



#endif //IFB_CONFIG_HPP