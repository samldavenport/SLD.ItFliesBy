#ifndef PLATFORM_INTERNAL_HPP
#define PLATFORM_INTERNAL_HPP

#include "sld.hpp"
#include "sld-opengl.hpp"
#include "ifb-input.hpp"
#include "ifb-types.hpp"
#include "eng-platform.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------

    typedef u32   pfm_window_event_flags;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct pfm_context;
    struct pfm_window_context;

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
};

#endif //PLATFORM_INTERNAL_HPP
