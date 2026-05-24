#pragma once

#include "win32.hpp"

namespace ifb {

    IFB_PLATFORM_API void
    pfm_graphics_init_opengl(
        gl_context* ctx) {

        win32_window&  window      = win32_window_instance();
        color_rgba_u32 clear_color = 0x282828FF;

        RECT window_rect;
        GetWindowRect(window.handle, &window_rect);

        gl_context_init_win32             (ctx, window.handle);
        gl_context_enable_depth_buffering (ctx);
        gl_context_enable_smoothing       (ctx);
        gl_context_set_clear_color_rgba   (ctx, clear_color);
        gl_context_update_viewport        (ctx, 0,0, window_rect.right, window_rect.bottom);
    }
};