#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"
#include "ifb-input.hpp"

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

    IFB_ENGINE_API bool
    eng_input_is_key_down(
        const input_keycode keycode) {

        // the gui gets the keyboard when one of its windows is focused or a control is active
        // the key state is still tracked, so nothing is stuck down when the focus comes back
        const bool is_gui_focused = (
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ||
            ImGui::GetIO().WantCaptureKeyboard
        );
        if (is_gui_focused) return(false);

        const bool is_key_down = keyboard_is_key_down(keycode);
        return(is_key_down);
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

