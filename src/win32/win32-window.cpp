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
        window.quit_received = false;

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
    pfm_window_frame_start(
        void) {

        win32_window& window = win32_window_instance();

        gl_context_clear_viewport(&window.context);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }
     
    IFB_WIN32_API_FUNC void
    pfm_window_frame_render(
        void) {

        win32_window& window = win32_window_instance();

        // render imgui
        ImGui::Render();
        ImDrawData* imgui_draw_data = ImGui::GetDrawData();
        assert(imgui_draw_data);

        // draw graphics and swap buffers
        ImGui_ImplOpenGL3_RenderDrawData(imgui_draw_data);
        SwapBuffers(window.device_context);
    }

    IFB_WIN32_API_FUNC void
    pfm_window_process_events(
        void) {
 
        // window instance
        win32_window& window = win32_window_instance();

        MSG  msg;
        const UINT msg_filter_min = 0;
        const UINT msg_filter_max = 0;
        const UINT msg_remove     = PM_REMOVE;

        for (
            bool has_message =  PeekMessage(&msg, NULL, msg_filter_min, msg_filter_max, msg_remove);
                 has_message == true;
                 has_message =  PeekMessage(&msg, NULL, msg_filter_min, msg_filter_max, msg_remove)) {

            switch (msg.message) {
                
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN: {
                    const input_keycode key_code = win32_input_get_keycode(msg.wParam);
                    eng_input_set_key_down(key_code);
                } break;

                case WM_KEYUP:
                case WM_SYSKEYUP: {
                    const input_keycode key_code = win32_input_get_keycode(msg.wParam);
                    eng_input_set_key_up(key_code);
                } break;
                
                case WM_MOUSEMOVE: {
                    const u32 x = LOWORD(msg.lParam);   
                    const u32 y = HIWORD(msg.lParam);
                    eng_input_mouse_move(x, y);
                } break;
                
                case(WM_QUIT):
                case(WM_DESTROY):
                case(WM_CLOSE): {
                    window.quit_received = true;
                }

                default: break;
            }

            // handle the message
            (void)TranslateMessage (&msg);
            (void)DispatchMessage  (&msg);
        }
    }

    IFB_WIN32_API_FUNC void
    pfm_window_close(
        void) {

        PostQuitMessage(0);
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

        LRESULT result = ImGui_ImplWin32_WndProcHandler(
            handle,
            message,
            w_param,
            l_param            
        );
        if (result) {
            return(result);
        }

        switch(message) {
     
            case(WM_CLOSE): {
                DestroyWindow(handle);
            } break;

            case(WM_DESTROY): {
                PostQuitMessage(0);
                return(0);
            } break;

            case (WM_SIZE): {
                const u32 width  = LOWORD(l_param);   
                const u32 height = HIWORD(l_param);  
                eng_window_set_size(width,height);
            } break;
     
            case WM_MOVE: {
                const u32 x = LOWORD(l_param);   
                const u32 y = HIWORD(l_param);   
                eng_window_set_pos(x,y);
            } break;
            

            default: break;
        }

        result = DefWindowProc(
            handle,
            message,
            w_param,
            l_param
        );

        return(result);
    }

    IFB_PLATFORM_API bool
    pfm_window_quit_received(
        void) {

        auto& window = win32_window_instance();
        return(window.quit_received);

    }


};