#pragma once

#include "win32.hpp"

namespace ifb {

    IFB_PLATFORM_API void
    pfm_graphics_init_opengl(
        void) {

        win32_window& window = win32_window_instance();
    
        gl_context_init_win32(&window.context, window.handle);
    }
  

};