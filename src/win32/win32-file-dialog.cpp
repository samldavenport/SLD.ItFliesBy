#pragma once

#include "ifb-platform.hpp"
#include "win32.hpp"
#include <shobjidl.h>
#include "sld-strings.hpp"

namespace ifb {

    IFB_PLATFORM_API bool
    pfm_file_dialog_open(
        pfm_file_dialog& dlg) {

        HRESULT hr = 0;
 
        // cache the dialog
        auto win32_dlg = _pfm_ctx->dlg_file_open;
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
            const u32 wstr_len_desc = win32_util_cstr_to_wstr(cstr_len_desc, dlg.filter_type, wstr_size, wstr_filter_desc);
            assert(wstr_len_type != 0);
            assert(wstr_len_desc != 0);

            // set the filter
            COMDLG_FILTERSPEC file_type;
            file_type.pszName = wstr_filter_type;
            file_type.pszSpec = wstr_filter_desc;
           
            win32_dlg->SetFileTypes(1, &file_type);
        }
 
        // NOTE(SLD): this shouldn't matter since we have only one window in this context
        hr = win32_dlg->Show(NULL);
        
        // if we succeeded, get the result
        if (SUCCEEDED(hr)) {

            IShellItem* item = NULL;
            hr = win32_dlg->GetResult(&item);
        }
        // otherwise, ensure it failed because it was cancelled
        else {
            assert(hr == HRESULT_FROM_WIN32(ERROR_CANCELLED));
            return(false);
        }
        
    }
    
    IFB_PLATFORM_API bool pfm_file_dialog_save (pfm_file_dialog& dlg);
};
