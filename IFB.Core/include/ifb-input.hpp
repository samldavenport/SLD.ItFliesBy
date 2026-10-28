#ifndef IFB_INPUT_HPP
#define IFB_INPUT_HPP

#include "sld.hpp"

using namespace sld;

namespace ifb {

    using input_keycode     = u16;
    using input_mouse_flags = u16;

    struct input_mouse;
    struct input_keyboard;
    struct input_gamepad;
    struct input_gamepad_stick;
    struct input_gamepad_triggers;

    enum input_keycode_e {

        input_keycode_e_null         = 0xFFFF,       
        
        input_keycode_e_escape       = 0x0000,
        input_keycode_e_tab          = 0x0001,
        input_keycode_e_enter        = 0x0002,
        input_keycode_e_backspace    = 0x0003,
        input_keycode_e_space        = 0x0004,
        input_keycode_e_shift_left   = 0x0005,
        input_keycode_e_shift_right  = 0x0006,
        input_keycode_e_ctrl_left    = 0x0007,
        input_keycode_e_ctrl_right   = 0x0008,
        input_keycode_e_alt_left     = 0x0009,
        input_keycode_e_alt_right    = 0x000A,
        input_keycode_e_caps_lock    = 0x000B,
        input_keycode_e_num_lock     = 0x000C,
        input_keycode_e_scroll_lock  = 0x000D,
        input_keycode_e_super_left   = 0x000E,
        input_keycode_e_super_right  = 0x000F,
        input_keycode_e_menu         = 0x0010,
        input_keycode_e_f1           = 0x0011,
        input_keycode_e_f2           = 0x0012,
        input_keycode_e_f3           = 0x0013,
        input_keycode_e_f4           = 0x0014,
        input_keycode_e_f5           = 0x0015,
        input_keycode_e_f6           = 0x0016,
        input_keycode_e_f7           = 0x0017,
        input_keycode_e_f8           = 0x0018,
        input_keycode_e_f9           = 0x0019,
        input_keycode_e_f10          = 0x001A,
        input_keycode_e_f11          = 0x001B,
        input_keycode_e_f12          = 0x001C,
        input_keycode_e_insert       = 0x001D,
        input_keycode_e_delete       = 0x001E,
        input_keycode_e_home         = 0x001F,
       
        input_keycode_e_end          = 0x0100,
        input_keycode_e_page_up      = 0x0101,
        input_keycode_e_page_down    = 0x0102,
        input_keycode_e_arrow_up     = 0x0103,
        input_keycode_e_arrow_down   = 0x0104,
        input_keycode_e_arrow_left   = 0x0105,
        input_keycode_e_arrow_right  = 0x0106,
        input_keycode_e_1            = 0x0107,
        input_keycode_e_2            = 0x0108,
        input_keycode_e_3            = 0x0109,
        input_keycode_e_4            = 0x010A,
        input_keycode_e_5            = 0x010B,
        input_keycode_e_6            = 0x010C,
        input_keycode_e_7            = 0x010D,
        input_keycode_e_8            = 0x010E,
        input_keycode_e_9            = 0x010F,
        input_keycode_e_0            = 0x0110,
        input_keycode_e_minus        = 0x0111,
        input_keycode_e_equals       = 0x0112,
        input_keycode_e_a            = 0x0113,
        input_keycode_e_b            = 0x0114,
        input_keycode_e_c            = 0x0115,
        input_keycode_e_d            = 0x0116,
        input_keycode_e_e            = 0x0117,
        input_keycode_e_f            = 0x0118,
        input_keycode_e_g            = 0x0119,
        input_keycode_e_h            = 0x011A,
        input_keycode_e_i            = 0x011B,
        input_keycode_e_j            = 0x011C,
        input_keycode_e_k            = 0x011D,
        input_keycode_e_l            = 0x011E,
        input_keycode_e_m            = 0x011F,

        input_keycode_e_n            = 0x0200,
        input_keycode_e_o            = 0x0201,
        input_keycode_e_p            = 0x0202,
        input_keycode_e_q            = 0x0203,
        input_keycode_e_r            = 0x0204,
        input_keycode_e_s            = 0x0205,
        input_keycode_e_t            = 0x0206,
        input_keycode_e_u            = 0x0207,
        input_keycode_e_v            = 0x0208,
        input_keycode_e_w            = 0x0209,
        input_keycode_e_x            = 0x020A,
        input_keycode_e_y            = 0x020B,
        input_keycode_e_z            = 0x020C,
        input_keycode_e_grave        = 0x020D, /*  `  */
        input_keycode_e_bracket_left = 0x020E, /*  [  */
        input_keycode_e_bracket_right= 0x020F, /*  ]  */
        input_keycode_e_backslash    = 0x0210, /*  \  */
        input_keycode_e_semicolon    = 0x0211, /*  ;  */
        input_keycode_e_apostrophe   = 0x0212, /*  '  */
        input_keycode_e_comma        = 0x0213, /*  ,  */
        input_keycode_e_period       = 0x0214, /*  .  */
        input_keycode_e_slash        = 0x0215, /*  /  */
        input_keycode_e_kp_divide    = 0x0216,
        input_keycode_e_kp_multiply  = 0x0217,
        input_keycode_e_kp_subtract  = 0x0218,
        input_keycode_e_kp_add       = 0x0219,
        input_keycode_e_kp_enter     = 0x021A,
        input_keycode_e_kp_decimal   = 0x021B,
        input_keycode_e_kp_0         = 0x021C,
        input_keycode_e_kp_1         = 0x021D,
        input_keycode_e_kp_2         = 0x021E,
        input_keycode_e_kp_3         = 0x021F,
        
        input_keycode_e_kp_4         = 0x0300,
        input_keycode_e_kp_5         = 0x0301,
        input_keycode_e_kp_6         = 0x0302,
        input_keycode_e_kp_7         = 0x0303,
        input_keycode_e_kp_8         = 0x0304,
        input_keycode_e_kp_9         = 0x0305,
        input_keycode_e_volume_mute  = 0x0306,
        input_keycode_e_volume_down  = 0x0307,
        input_keycode_e_volume_up    = 0x0308,
        input_keycode_e_media_next   = 0x0309,
        input_keycode_e_media_prev   = 0x030A,
        input_keycode_e_media_stop   = 0x030B,
        input_keycode_e_media_play   = 0x030C,
    };

    enum input_gamepad_button_flag_e {
        input_gamepad_button_flag_e_start        = bit_value(0),
        input_gamepad_button_flag_e_select       = bit_value(1),
        input_gamepad_button_flag_e_bumper_left  = bit_value(2),
        input_gamepad_button_flag_e_bumper_right = bit_value(3),
        input_gamepad_button_flag_e_stick_left   = bit_value(4),
        input_gamepad_button_flag_e_stick_right  = bit_value(5),
        input_gamepad_button_flag_e_dpad_up      = bit_value(6),
        input_gamepad_button_flag_e_dpad_down    = bit_value(7),
        input_gamepad_button_flag_e_dpad_left    = bit_value(8),
        input_gamepad_button_flag_e_dpad_right   = bit_value(9),
        input_gamepad_button_flag_e_bpad_up      = bit_value(10),
        input_gamepad_button_flag_e_bpad_down    = bit_value(11),
        input_gamepad_button_flag_e_bpad_left    = bit_value(12),
        input_gamepad_button_flag_e_bpad_right   = bit_value(13)
    };

    enum input_mouse_flag_e {
        input_mouse_flag_e_click_left   = bit_value(0),
        input_mouse_flag_e_click_right  = bit_value(1),
        input_mouse_flag_e_click_middle = bit_value(2),
        input_mouse_flag_e_scroll       = bit_value(3),
        input_mouse_flag_e_move         = bit_value(4)
    };

    struct input_mouse {
        f32               scroll;
        u16               x;
        u16               y;
        input_mouse_flags flags;
    };
};

#endif //IFB_INPUT_HPP
