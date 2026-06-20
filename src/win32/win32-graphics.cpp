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

        const u32 viewport_width  = (window_rect.right  - window_rect.left);
        const u32 viewport_height = (window_rect.bottom - window_rect.top);

        gl_context_init_win32             (ctx, window.handle);
        gl_context_enable_depth_buffering (ctx);
        gl_context_enable_smoothing       (ctx);
        gl_context_set_clear_color_rgba   (ctx, clear_color);
        gl_context_update_viewport        (ctx, 0,0, viewport_width, viewport_height);
    }

    IFB_PLATFORM_API void
    pfm_graphics_init_imgui(
        void) {

        // initialize context
        ImGuiContext* imgui_context = IMGUI_CHECKVERSION() ? ImGui::CreateContext() : NULL;  
        assert(imgui_context);

        // initialize win32/opengl methods
        win32_window&  window = win32_window_instance();
        bool did_init_impl = true;
        did_init_impl &= ImGui_ImplWin32_InitForOpenGL (window.handle);
        did_init_impl &= ImGui_ImplOpenGL3_Init        ("#version 330");
        assert(did_init_impl);
    }

};