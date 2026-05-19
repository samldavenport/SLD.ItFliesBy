#ifndef IFB_PLATFORM_HPP
#define IFB_PLATFORM_HPP

#include "sld.hpp"

using namespace sld;

namespace ifb {

    typedef u32 window_event_flags;

    struct window_context;
    struct window_config;
    struct window_position;
    struct window_size;

    void window_open  (window_context* ctx, const window_config* cfg);
    void window_close (window_context* ctx);

    struct window_context {
        window_event_flags event_flags;
    };

    enum window_event_flag_e {
        window_event_flag_e_none              = 0;
        window_event_flag_e_close             = bit_value(1),
        window_event_flag_e_minimize          = bit_value(2),
        window_event_flag_e_position_update   = bit_value(3),
        window_event_flag_e_size_update       = bit_value(4),
        window_event_flag_e_key_down          = bit_value(5),
        window_event_flag_e_key_up            = bit_value(6),
        window_event_flag_e_mouse_click_left  = bit_value(7),
        window_event_flag_e_mouse_click_right = bit_value(8),
        window_event_flag_e_mouse_move        = bit_value(9),
        window_event_flag_e_mouse_scroll      = bit_value(10)
    };

};

#endif //IFB_PLATFORM_HPP