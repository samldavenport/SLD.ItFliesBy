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
        assert(w_str_size != 0);
        assert(w_str_ptr  != NULL);

        const u32 w_str_len = MultiByteToWideChar(
            CP_UTF8,
            NULL,
            c_str_ptr,
            c_str_len,
            w_str_ptr,
            w_str_size
        );

        return(w_str_len);
    }
};
