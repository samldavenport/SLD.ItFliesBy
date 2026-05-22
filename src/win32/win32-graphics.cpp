#pragma once

#include "win32.hpp"

namespace ifb {

    IFB_PLATFORM_API void
    pfm_graphics_init_opengl(
        void) {

        color_rgba_u32 clear_color;
        clear_color.hex = 0x282828FF;
        win32_window& window = win32_window_instance();


        RECT window_rect;
        GetWindowRect(window.handle, &window_rect);

        gl_context_init_win32             (&window.context, window.handle);
        gl_context_enable_depth_buffering (&window.context);
        gl_context_enable_smoothing       (&window.context);
        gl_context_set_clear_color_rgba   (&window.context, clear_color);
        gl_context_update_viewport        (&window.context, 0,0, window_rect.right, window_rect.bottom);
    }
};