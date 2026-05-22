#ifndef IFB_INPUT_HPP
#define IFB_INPUT_HPP

#include "ifb.hpp"
#include "ifb-config.hpp"

namespace ifb {

    using input_keycode     = u16;
    using input_mouse_flags = u16;

    struct input_mouse;
    struct input_keyboard;
    struct input_gamepad;
    struct input_gamepad_stick;
    struct input_gamepad_triggers;

    enum input_keycode_e {
        
        // control
        input_keycode_e_null         = 0x00,
        input_keycode_e_escape       = 0x01,  // esc
        input_keycode_e_tab          = 0x0F,
        input_keycode_e_enter        = 0x1C,
        input_keycode_e_backspace    = 0x0E,
        input_keycode_e_space        = 0x39,

        // modifiers
        input_keycode_e_shift_left   = 0x2A,
        input_keycode_e_shift_right  = 0x36,
        input_keycode_e_ctrl_left    = 0x1D,
        input_keycode_e_ctrl_right   = 0x11D,
        input_keycode_e_alt_left     = 0x38,
        input_keycode_e_alt_right    = 0x138,
        input_keycode_e_caps_lock    = 0x3A,
        input_keycode_e_num_lock     = 0x45,
        input_keycode_e_scroll_lock  = 0x46,
        input_keycode_e_super_left   = 0x15B, // win / command
        input_keycode_e_super_right  = 0x15C,
        input_keycode_e_menu         = 0x15D,

        // function keys
        input_keycode_e_f1           = 0x3B,
        input_keycode_e_f2           = 0x3C,
        input_keycode_e_f3           = 0x3D,
        input_keycode_e_f4           = 0x3E,
        input_keycode_e_f5           = 0x3F,
        input_keycode_e_f6           = 0x40,
        input_keycode_e_f7           = 0x41,
        input_keycode_e_f8           = 0x42,
        input_keycode_e_f9           = 0x43,
        input_keycode_e_f10          = 0x44,
        input_keycode_e_f11          = 0x57,
        input_keycode_e_f12          = 0x58,

        // navigation cluster
        input_keycode_e_insert       = 0x152,
        input_keycode_e_delete       = 0x153,
        input_keycode_e_home         = 0x147,
        input_keycode_e_end          = 0x14F,
        input_keycode_e_page_up      = 0x149,
        input_keycode_e_page_down    = 0x151,
        input_keycode_e_arrow_up     = 0x148,
        input_keycode_e_arrow_down   = 0x150,
        input_keycode_e_arrow_left   = 0x14B,
        input_keycode_e_arrow_right  = 0x14D,

        // number row (shifted versions are symbols)
        input_keycode_e_1            = 0x02,
        input_keycode_e_2            = 0x03,
        input_keycode_e_3            = 0x04,
        input_keycode_e_4            = 0x05,
        input_keycode_e_5            = 0x06,
        input_keycode_e_6            = 0x07,
        input_keycode_e_7            = 0x08,
        input_keycode_e_8            = 0x09,
        input_keycode_e_9            = 0x0A,
        input_keycode_e_0            = 0x0B,
        input_keycode_e_minus        = 0x0C, // -
        input_keycode_e_equals       = 0x0D, // =

        // letters
        input_keycode_e_a            = 0x1E,
        input_keycode_e_b            = 0x30,
        input_keycode_e_c            = 0x2E,
        input_keycode_e_d            = 0x20,
        input_keycode_e_e            = 0x12,
        input_keycode_e_f            = 0x21,
        input_keycode_e_g            = 0x22,
        input_keycode_e_h            = 0x23,
        input_keycode_e_i            = 0x17,
        input_keycode_e_j            = 0x24,
        input_keycode_e_k            = 0x25,
        input_keycode_e_l            = 0x26,
        input_keycode_e_m            = 0x32,
        input_keycode_e_n            = 0x31,
        input_keycode_e_o            = 0x18,
        input_keycode_e_p            = 0x19,
        input_keycode_e_q            = 0x10,
        input_keycode_e_r            = 0x13,
        input_keycode_e_s            = 0x1F,
        input_keycode_e_t            = 0x14,
        input_keycode_e_u            = 0x16,
        input_keycode_e_v            = 0x2F,
        input_keycode_e_w            = 0x11,
        input_keycode_e_x            = 0x2D,
        input_keycode_e_y            = 0x15,
        input_keycode_e_z            = 0x2C,
        
        // punctuation
        input_keycode_e_grave        = 0x29, /*  `  */
        input_keycode_e_bracket_left = 0x1A, /*  [  */
        input_keycode_e_bracket_right= 0x1B, /*  ]  */
        input_keycode_e_backslash    = 0x2B, /*  \  */
        input_keycode_e_semicolon    = 0x27, /*  ;  */
        input_keycode_e_apostrophe   = 0x28, /*  '  */
        input_keycode_e_comma        = 0x33, /*  ,  */
        input_keycode_e_period       = 0x34, /*  .  */
        input_keycode_e_slash        = 0x35, /*  /  */

        // keypad
        input_keycode_e_kp_divide    = 0x135,
        input_keycode_e_kp_multiply  = 0x37,
        input_keycode_e_kp_subtract  = 0x4A,
        input_keycode_e_kp_add       = 0x4E,
        input_keycode_e_kp_enter     = 0x11C,
        input_keycode_e_kp_decimal   = 0x53,
        input_keycode_e_kp_0         = 0x52,
        input_keycode_e_kp_1         = 0x4F,
        input_keycode_e_kp_2         = 0x50,
        input_keycode_e_kp_3         = 0x51,
        input_keycode_e_kp_4         = 0x4B,
        input_keycode_e_kp_5         = 0x4C,
        input_keycode_e_kp_6         = 0x4D,
        input_keycode_e_kp_7         = 0x47,
        input_keycode_e_kp_8         = 0x48,
        input_keycode_e_kp_9         = 0x49,

        // multimedia
        input_keycode_e_volume_mute  = 0x120,
        input_keycode_e_volume_down  = 0x12E,
        input_keycode_e_volume_up    = 0x130,
        input_keycode_e_media_next   = 0x119,
        input_keycode_e_media_prev   = 0x110,
        input_keycode_e_media_stop   = 0x124,
        input_keycode_e_media_play   = 0x122,
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

    struct input_keycode_array {
        u16           count;
        input_keycode keycodes[IFB_CONFIG_WINDOW_MAX_KEYS];
    };

    struct input_keyboard {
        input_keycode_array array_up;
        input_keycode_array array_down;
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