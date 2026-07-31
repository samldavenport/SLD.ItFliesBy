#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_input_set_key_down(
        const input_keycode key) {

        keyboard_set_key_down(key);
    }

    IFB_ENGINE_API void
    eng_input_set_key_up(
        const input_keycode key) {

        keyboard_set_key_up(key);
    }

    IFB_ENGINE_API void
    eng_input_mouse_click_left_down(
        void) {

    }

    IFB_ENGINE_API void
    eng_input_mouse_click_left_up(
        void) {

    }

    IFB_ENGINE_API void
    eng_input_mouse_click_right_down(
        void) {

    }

    IFB_ENGINE_API void
    eng_input_mouse_click_right_up(
        void) {

    }

    IFB_ENGINE_API void
    eng_input_mouse_move(
        const u32 x,
        const u32 y) {

    }
};

