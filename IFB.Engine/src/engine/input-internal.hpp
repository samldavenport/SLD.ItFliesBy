#ifndef INPUT_INTERNAL_HPP
#define INPUT_INTERNAL_HPP

#include "ifb-input.hpp"
#include "eng-input.hpp"
#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    using input_mouse_flags = u16;
    struct input_mouse;
    struct input_keyboard;
    struct input_gamepad;
    struct input_gamepad_stick;
    struct input_gamepad_triggers;
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

#endif //INPUT_INTERNAL_HPP
