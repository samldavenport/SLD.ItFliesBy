#ifndef WIN32_HPP
#define WIN32_HPP

#include <Windows.h>
#include "ifb.hpp"

#define IFB_WIN32_API_FUNC
#define IFB_WIN32_INTERNAL static

namespace ifb {

    struct win32_window {
        LPWNDCLASS  class_type;
        HWND        handle;
        HDC         device_context;
        gl_context  context;
    };

    inline win32_window&
    win32_window_instance(void) {
        static win32_window window;
        return(window);
    }

    LPWNDCLASS win32_window_class_instance(void);    
    
    LRESULT CALLBACK win32_window_callback(
        HWND   handle,
        UINT   message,
        WPARAM w_param,
        LPARAM l_param
    );

    input_keycode
    win32_input_get_keycode(
        const DWORD wparam);
    
};

#endif //WIN32_HPP