#ifndef IFB_PLATFORM_HPP
#define IFB_PLATFORM_HPP

#include "sld.hpp"

using namespace sld;

namespace ifb {

    static const u32 MAX_KEYS       = 8;
    static const u32 STR_SIZE_TITLE = 32;

    typedef u32 pfm_window_event_flags;
    typedef u32 pfm_window_key_code;

    struct pfm_window_context;
    struct pfm_window_config;

    struct pfm_window_config {
        cchar* title;
        struct  {
            u32 x;
            u32 y;
            u32 width;
            u32 height;
        } init_dims;
    };

    void pfm_window_open  (pfm_window_context* ctx, const pfm_window_config* cfg);
    void pfm_window_close (pfm_window_context* ctx);

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
        struct {
            u32                 key_count_up;
            u32                 key_count_down;
            pfm_window_key_code key_codes_up   [MAX_KEYS];
            pfm_window_key_code key_codes_down [MAX_KEYS];
        } keys;
    };

    enum window_event_flag_e {
        window_event_flag_e_none              = 0;
        window_event_flag_e_close             = bit_value(1),
        window_event_flag_e_minimize          = bit_value(2),
        window_event_flag_e_maximize          = bit_value(3),
        window_event_flag_e_position_update   = bit_value(4),
        window_event_flag_e_size_update       = bit_value(5),
        window_event_flag_e_key_down          = bit_value(6),
        window_event_flag_e_key_up            = bit_value(7),
        window_event_flag_e_mouse_click_left  = bit_value(8),
        window_event_flag_e_mouse_click_right = bit_value(9),
        window_event_flag_e_mouse_move        = bit_value(10),
        window_event_flag_e_mouse_scroll      = bit_value(11)
    };

};

#endif //IFB_PLATFORM_HPP