#ifndef IFB_WIN32_HPP
#define IFB_WIN32_HPP

#include <Windows.h>
#include <ShObjIdl.h>
#include "ifb.hpp"
#include "ifb-input.hpp"
#include "eng-platform.hpp"

#define IFB_WIN32_API_FUNC
#define IFB_WIN32_INTERNAL static

namespace ifb {

    struct pfm_context {
        IFileOpenDialog* dlg_file_open;
        IFileSaveDialog* dlg_file_save;
    } static _pfm_ctx;

    struct win32_window {
        LPWNDCLASS  class_type;
        HWND        handle;
        HDC         device_context;
        gl_context  context;
        b8          quit_received;
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
        const WPARAM wparam,
        const LPARAM lparam);
   
    u32 
    win32_util_cstr_to_wstr(
        const u32    c_str_len,
        const cchar* c_str_ptr,
        const u32    w_str_size,
        wchar*       w_str_ptr
    );

    u32
    win32_util_wstr_to_cstr(
        const u32    w_str_len,
        const wchar* w_str_ptr,
        const u32    c_str_size,
        cchar*       c_str_ptr
    );
};

#endif //IFB_WIN32_HPP
