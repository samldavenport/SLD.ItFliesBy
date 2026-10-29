#pragma once

#include "platform-internal.hpp"
#include "ifb-win32.hpp"
#include <shobjidl.h>
#include "sld-strings.hpp"
#include "sld.hpp"
#include "eng-platform.hpp"

namespace ifb {

    IFB_PLATFORM_API bool
    pfm_file_dialog_open(
        pfm_file_dialog& dlg) {

        HRESULT hr = 0;
 
        // the file types can only be set once per dialog, so every call gets a new one
        if (_pfm_ctx.dlg_file_open) {
            _pfm_ctx.dlg_file_open->Release();
            _pfm_ctx.dlg_file_open = NULL;
        }
        hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&_pfm_ctx.dlg_file_open)
        );
        assert(SUCCEEDED(hr));

        auto win32_dlg = _pfm_ctx.dlg_file_open;
        assert(win32_dlg);

        // NOTE(SLD): I cant think of a scenario where
        // we would need longer wide strings if we have
        // a single file type/desc
        const u32 wstr_size = 32;
        wchar wstr_filter_type[wstr_size];
        wchar wstr_filter_desc[wstr_size];

        const bool use_filters = dlg.filter_type != NULL && dlg.filter_desc != NULL;
        if (use_filters) {

            // calculate the cstr length
            const u32 cstr_len_type = cstr_nvar_length(dlg.filter_type, 16); 
            const u32 cstr_len_desc = cstr_nvar_length(dlg.filter_desc, 16); 
           
            // convert filter cstr to wstr
            const u32 wstr_len_type = win32_util_cstr_to_wstr(cstr_len_type, dlg.filter_type, wstr_size, wstr_filter_type);
            const u32 wstr_len_desc = win32_util_cstr_to_wstr(cstr_len_desc, dlg.filter_desc, wstr_size, wstr_filter_desc);
            assert(wstr_len_type != 0);
            assert(wstr_len_desc != 0);

            // set the filter
            COMDLG_FILTERSPEC file_type;
            file_type.pszName = wstr_filter_desc;
            file_type.pszSpec = wstr_filter_type;

            hr = win32_dlg->SetFileTypes(1, &file_type);
            assert(SUCCEEDED(hr));
        }

        // NOTE(SLD): this shouldn't matter since we have only one window in this context
        hr = win32_dlg->Show(NULL);

        // if we failed, ensure it was because the dialog was cancelled
        if (FAILED(hr)) {
            assert(hr == HRESULT_FROM_WIN32(ERROR_CANCELLED));
            return(false);
        }

        // otherwise, get the result
        IShellItem* item = NULL;
        hr = win32_dlg->GetResult(&item);
        if (FAILED(hr) || item == NULL) {
            return(false);
        }

        static cchar cstr_result_path[256];
        zero_memory((void*)cstr_result_path,sizeof(cstr_result_path));

        // get the selected path, we own this string and must free it
        PWSTR wstr_item_path = NULL;
        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &wstr_item_path);
        item->Release();
        if (FAILED(hr) || wstr_item_path == NULL) {
            return(false);
        }

        // copy the path into the result
        const u32 wstr_len_path = lstrlenW(wstr_item_path);
        const u32 cstr_len_path = (wstr_len_path != 0)
            ? win32_util_wstr_to_cstr(wstr_len_path, wstr_item_path, sizeof(cstr_result_path), cstr_result_path)
            : 0;
        CoTaskMemFree(wstr_item_path);

        // a length of 0 means the path didn't fit
        assert(cstr_len_path != 0);

        dlg.path = cstr_result_path;
        return(true);
    }
    
    IFB_PLATFORM_API bool
    pfm_file_dialog_save(
        pfm_file_dialog& dlg) {

        HRESULT hr = 0;

        // the file types can only be set once per dialog, so every call gets a new one
        if (_pfm_ctx.dlg_file_save) {
            _pfm_ctx.dlg_file_save->Release();
            _pfm_ctx.dlg_file_save = NULL;
        }
        hr = CoCreateInstance(
            CLSID_FileSaveDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&_pfm_ctx.dlg_file_save)
        );
        assert(SUCCEEDED(hr));

        auto win32_dlg = _pfm_ctx.dlg_file_save;
        assert(win32_dlg);

        // NOTE(SLD): I cant think of a scenario where
        // we would need longer wide strings if we have
        // a single file type/desc
        const u32 wstr_size = 32;
        wchar wstr_filter_type[wstr_size];
        wchar wstr_filter_desc[wstr_size];

        const bool use_filters = dlg.filter_type != NULL && dlg.filter_desc != NULL;
        if (use_filters) {

            // calculate the cstr length
            const u32 cstr_len_type = cstr_nvar_length(dlg.filter_type, 16);
            const u32 cstr_len_desc = cstr_nvar_length(dlg.filter_desc, 16);

            // convert filter cstr to wstr
            const u32 wstr_len_type = win32_util_cstr_to_wstr(cstr_len_type, dlg.filter_type, wstr_size, wstr_filter_type);
            const u32 wstr_len_desc = win32_util_cstr_to_wstr(cstr_len_desc, dlg.filter_desc, wstr_size, wstr_filter_desc);
            assert(wstr_len_type != 0);
            assert(wstr_len_desc != 0);

            // set the filter
            COMDLG_FILTERSPEC file_type;
            file_type.pszName = wstr_filter_desc;
            file_type.pszSpec = wstr_filter_type;

            hr = win32_dlg->SetFileTypes(1, &file_type);
            assert(SUCCEEDED(hr));
        }

        // NOTE(SLD): this shouldn't matter since we have only one window in this context
        hr = win32_dlg->Show(NULL);

        // if we failed, ensure it was because the dialog was cancelled
        if (FAILED(hr)) {
            assert(hr == HRESULT_FROM_WIN32(ERROR_CANCELLED));
            return(false);
        }

        // otherwise, get the result
        IShellItem* item = NULL;
        hr = win32_dlg->GetResult(&item);
        if (FAILED(hr) || item == NULL) {
            return(false);
        }

        static cchar cstr_result_path[256];
        zero_memory((void*)cstr_result_path,sizeof(cstr_result_path));

        // get the selected path, we own this string and must free it
        PWSTR wstr_item_path = NULL;
        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &wstr_item_path);
        item->Release();
        if (FAILED(hr) || wstr_item_path == NULL) {
            return(false);
        }

        // copy the path into the result
        const u32 wstr_len_path = lstrlenW(wstr_item_path);
        const u32 cstr_len_path = (wstr_len_path != 0)
            ? win32_util_wstr_to_cstr(wstr_len_path, wstr_item_path, sizeof(cstr_result_path), cstr_result_path)
            : 0;
        CoTaskMemFree(wstr_item_path);

        // a length of 0 means the path didn't fit
        assert(cstr_len_path != 0);

        dlg.path = cstr_result_path;
        return(true);
    }
};
