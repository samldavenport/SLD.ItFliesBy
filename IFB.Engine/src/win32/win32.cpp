#pragma once

#include "ifb-win32.hpp"
#include <Windows.h>
#include "win32-audio.cpp"
#include "win32-file.cpp"
#include "win32-file-dialog.cpp"
#include "win32-graphics.cpp"
#include "win32-input.cpp"
#include "win32-memory.cpp"
#include "win32-monitor.cpp"
#include "win32-system.cpp"
#include "win32-window.cpp"
#include "win32-utils.cpp"
#include "win32-thread.cpp"
#include "ifb-platform.hpp"

namespace ifb {
    
    IFB_PLATFORM_API void
    pfm_context_startup(void) {

        HRESULT hr = 0;

        // initialize COM library
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); 
        assert(SUCCEEDED(hr));

        // initialize file open dialog
        hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&_pfm_ctx.dlg_file_open)
        );
        assert(SUCCEEDED(hr));
    
        // intialize the file save dialog 
        hr = CoCreateInstance(
            CLSID_FileSaveDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&_pfm_ctx.dlg_file_save)
        );
        assert(SUCCEEDED(hr));

        // initialize the thread table
        for (
            u32 i_thread = 0;
                i_thread < config::thread_capacity;
              ++i_thread) {

            auto& thread = _pfm_ctx.threads[i_thread];
            thread.proc = NULL;
            thread.ctx  = NULL;
            thread.hnd  = NULL;
        }
    } 
    
    IFB_PLATFORM_API void
    pfm_context_shutdown(
        void) {

        auto dlg_open = _pfm_ctx.dlg_file_open;
        auto dlg_save = _pfm_ctx.dlg_file_save;
       
        if (dlg_open) dlg_open->Release();
        if (dlg_save) dlg_save->Release();

        _pfm_ctx.dlg_file_open = NULL;
        _pfm_ctx.dlg_file_save = NULL;
    
        CoUninitialize();
    }
};
