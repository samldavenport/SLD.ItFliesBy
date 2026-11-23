#pragma once

#include "ifb.hpp"
#include "eng-context.hpp"
#include "ifb-input.hpp"
#include "eng-renderer.hpp"

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

    // for when the window loses the focus, like to a debugger on a breakpoint
    // the key ups go to whatever has the focus, so without this a held key stays down
    IFB_ENGINE_API void
    eng_input_release_all_keys(
        void) {

        keyboard_input_reset();
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
    
    //--------------------------------------------------------------------
    // MOUSE
    //--------------------------------------------------------------------

    // the mouse in window pixels, from the top left of the area that's drawn to
    // it's only ever moved by the window, so it stays put while the mouse is outside it
    struct input_mouse_state {
        s32  x;
        s32  y;
        bool has_moved;      // false until the first move, so nothing reads a made up position
        bool is_left_down;
        bool is_right_down;
    } static _mouse;

    // the gui gets the mouse while it's over one of its windows, or dragging something that started in one
    inline bool
    input_mouse_is_gui_captured(
        void) {

        return(ImGui::GetIO().WantCaptureMouse);
    }

    IFB_ENGINE_API void
    eng_input_mouse_click_left_down(
        void) {

        _mouse.is_left_down = true;
    }

    IFB_ENGINE_API void
    eng_input_mouse_click_left_up(
        void) {

        _mouse.is_left_down = false;
    }

    IFB_ENGINE_API void
    eng_input_mouse_click_right_down(
        void) {

        _mouse.is_right_down = true;
    }

    IFB_ENGINE_API void
    eng_input_mouse_click_right_up(
        void) {

        _mouse.is_right_down = false;
    }

    // window pixels, it can be outside the window while a drag holds the mouse
    IFB_ENGINE_API void
    eng_input_mouse_move(
        const s32 x,
        const s32 y) {

        _mouse.x         = x;
        _mouse.y         = y;
        _mouse.has_moved = true;
    }

    // for when the window loses the focus, the button ups go to whatever has it
    IFB_ENGINE_API void
    eng_input_mouse_release_all(
        void) {

        _mouse.is_left_down  = false;
        _mouse.is_right_down = false;
    }

    IFB_ENGINE_API bool
    eng_input_mouse_is_left_down(
        void) {

        if (input_mouse_is_gui_captured()) return(false);
        return(_mouse.is_left_down);
    }

    IFB_ENGINE_API bool
    eng_input_mouse_is_right_down(
        void) {

        if (input_mouse_is_gui_captured()) return(false);
        return(_mouse.is_right_down);
    }

    // where the mouse is on the screen layer, in the units eng_draw_*_screen takes
    // false while the gui has the mouse, or before it's ever moved over the window
    IFB_ENGINE_API bool
    eng_input_mouse_get_screen_pos(
        vec2& pos) {

        if (!_mouse.has_moved)             return(false);
        if (input_mouse_is_gui_captured()) return(false);

        u32 window_width  = 0;
        u32 window_height = 0;
        u32 screen_width  = 0;
        u32 screen_height = 0;
        renderer_get_viewport_size (window_width, window_height);
        renderer_get_screen_size   (screen_width, screen_height);
        if (window_width == 0 || window_height == 0) return(false);

        // the screen layer is stretched over the whole window
        pos.x = (f32)_mouse.x * ((f32)screen_width  / (f32)window_width);
        pos.y = (f32)_mouse.y * ((f32)screen_height / (f32)window_height);
        return(true);
    }
};

