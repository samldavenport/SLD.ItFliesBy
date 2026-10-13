#include "win32.hpp"

namespace ifb {

    u32 
    win32_util_cstr_to_wstr(
        const u32    c_str_len,
        const cchar* c_str_ptr,
        const u32    w_str_size,
        wchar*       w_str_ptr) {

        assert(c_str_len  != 0);
        assert(c_str_ptr  != NULL);
        assert(w_str_size >  1);
        assert(w_str_ptr  != NULL);

        // NOTE(SLD): an explicit length means the result is NOT
        // null terminated, so we reserve a character and do it ourselves
        const u32 w_str_len = MultiByteToWideChar(
            CP_UTF8,
            NULL,
            c_str_ptr,
            c_str_len,
            w_str_ptr,
            w_str_size - 1
        );
        w_str_ptr[w_str_len] = 0;

        return(w_str_len);
    }

    u32
    win32_util_wstr_to_cstr(
        const u32    w_str_len,
        const wchar* w_str_ptr,
        const u32    c_str_size,
        cchar*       c_str_ptr) {

        assert(w_str_len  != 0);
        assert(w_str_ptr  != NULL);
        assert(c_str_size >  1);
        assert(c_str_ptr  != NULL);

        // NOTE(SLD): an explicit length means the result is NOT
        // null terminated, so we reserve a character and do it ourselves
        const u32 c_str_len = WideCharToMultiByte(
            CP_UTF8,
            NULL,
            w_str_ptr,
            w_str_len,
            c_str_ptr,
            c_str_size - 1,
            NULL,
            NULL
        );
        c_str_ptr[c_str_len] = 0;

        return(c_str_len);
    }
};
