#pragma once

#include "win32.hpp"


namespace ifb {

    static win32_window _window;

    //--------------------------------------------------------------------
    // PLATFORM API METHODS
    //--------------------------------------------------------------------

    IFB_WIN32_API_FUNC void
    pfm_window_open(
        const pfm_window_config* cfg) {

        // window instance
        win32_window& window = win32_window_instance();

        // window class
        window.class_type  = win32_window_class_instance();
        assert(window.class_type);

        // create window handle
        window.handle = CreateWindow(
            window.class_type->lpszClassName,
            cfg->title,
            WS_OVERLAPPEDWINDOW,
            cfg->init_dims.x,
            cfg->init_dims.y,
            cfg->init_dims.width,
            cfg->init_dims.height,
            NULL,
            NULL,
            window.class_type->hInstance,
            NULL
        );
        assert(window.handle);

        // create device context
        window.device_context = GetDC(window.handle);
        assert(window.device_context);

        // show window
        (void)ShowWindow(window.handle, 1);
    }

    IFB_WIN32_API_FUNC void
    pfm_window_process_events(
        void) {
 
    }

    IFB_WIN32_API_FUNC void
    pfm_window_close(
        void) {

    }

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_WIN32_INTERNAL LPWNDCLASS
    win32_window_class_instance(
        void) {

        static WNDCLASS   window_class;
        static LPWNDCLASS window_class_ptr = NULL;

        if (!window_class_ptr) {

            // register the window class
            window_class.lpfnWndProc   = (WNDPROC)win32_window_callback;
            window_class.hInstance     = GetModuleHandle(NULL);  
            window_class.lpszClassName = "IFB Window";
            window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

            const DWORD result = RegisterClass(&window_class);
            window_class_ptr   = (result == 0)
                ? NULL 
                : &window_class;                
        }

        return(window_class_ptr);
    } 


    IFB_WIN32_INTERNAL LRESULT CALLBACK
    win32_window_callback(
        HWND   handle,
        UINT   message,
        WPARAM w_param,
        LPARAM l_param) {

        switch(message) {
     
            case(WM_CLOSE): {
                PostQuitMessage(0);
            } break;
     
            default: break;
        }

        LRESULT result = DefWindowProc(
            handle,
            message,
            w_param,
            l_param
        );

        return(result);
    }

};