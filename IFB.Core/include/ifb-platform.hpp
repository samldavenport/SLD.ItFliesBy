#ifndef IFB_PLATFORM_HPP
#define IFB_PLATFORM_HPP

#include "sld.hpp"
#include "sld-opengl.hpp"
#include "ifb-input.hpp"
#include "ifb-types.hpp"

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------

    typedef u32   pfm_window_event_flags;
    typedef u32   pfm_file_mode;
    typedef u32   pfm_file_access_flags;
    typedef u32   pfm_file_share_flags;
    typedef void* pfm_hnd_file;
 
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct pfm_context;
    struct pfm_window_context;
    struct pfm_window_config;
    struct pfm_file_config;
    struct pfm_file_buffer;
    struct pfm_file_dialog;
    struct pfm_monitor_info;
    struct pfm_monitor_area;
    struct pfm_memory_info;

    //--------------------------------------------------------------------
    // ENUMS
    //--------------------------------------------------------------------

    enum pfm_window_event_flag_e {
        pfm_window_event_flag_e_none              = 0,
        pfm_window_event_flag_e_close             = bit_value(1),
        pfm_window_event_flag_e_minimize          = bit_value(2),
        pfm_window_event_flag_e_maximize          = bit_value(3),
        pfm_window_event_flag_e_position_update   = bit_value(4),
        pfm_window_event_flag_e_size_update       = bit_value(5),
        pfm_window_event_flag_e_key_down          = bit_value(6),
        pfm_window_event_flag_e_key_up            = bit_value(7),
        pfm_window_event_flag_e_mouse_click_left  = bit_value(8),
        pfm_window_event_flag_e_mouse_click_right = bit_value(9),
        pfm_window_event_flag_e_mouse_move        = bit_value(10),
        pfm_window_event_flag_e_mouse_scroll      = bit_value(11)
    };

    enum pfm_file_access_flag_e {
        pfm_file_access_flag_e_none               = 0,        
        pfm_file_access_flag_e_read               = bit_value(0),        
        pfm_file_access_flag_e_write              = bit_value(1)
    };

    enum pfm_file_share_flag_e {
        pfm_file_share_flag_e_none                = 0,
        pfm_file_share_flag_e_read                = bit_value(0),
        pfm_file_share_flag_e_write               = bit_value(1),
        pfm_file_share_flag_e_delete              = bit_value(2)
    };

    enum pfm_file_mode_e {
        pfm_file_mode_e_create_new                = 0, 
        pfm_file_mode_e_open_existing             = 1, 
        pfm_file_mode_e_open_always               = 2, 
        pfm_file_mode_e_overwrite_existing        = 3 
    };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct pfm_window_config {
        cchar* title;
        struct  {
            u32 x;
            u32 y;
            u32 width;
            u32 height;
        } init_dims;
    };

    struct pfm_window_context {
        pfm_window_event_flags event_flags;
        struct {
            u32 width;
            u32 height;
            u32 x;
            u32 y;
        } dims;
        struct {
            u32 x;
            u32 y;
            f32 scroll;
        } mouse;
    };

    struct pfm_file_config {
        const cchar*          path;
        pfm_file_mode         mode;
        pfm_file_access_flags access_flags;
        pfm_file_share_flags  share_flags;
        bool                  is_async;
    };

    struct pfm_file_buffer {
        byte* data;
        u32   size;
        u32   length;
        u32   offset;
        u32   cursor;
    };

    struct pfm_file_dialog {
        const cchar* path;
        const cchar* filter_type; 
        const cchar* filter_desc; 
    };

    struct pfm_monitor_info {
        u32 index;
        u32 refresh_rate_hz;
        u32 pixel_width;
        u32 pixel_height;
        u32 x;
        u32 y;
    };

    struct pfm_monitor_area {
        u32 virtual_pixel_width;
        u32 virtual_pixel_height;
    };

    struct pfm_memory_info {
        u32 size_page;
        u32 size_granularity;
    };
};

#endif //IFB_PLATFORM_HPP
