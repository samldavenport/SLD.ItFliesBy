#pragma once

#include "win32.hpp"

namespace ifb {

    IFB_WIN32_API_FUNC void*
    pfm_memory_commit(
        const void* res,
        const u32   offset,
        const u32   size) {

        assert(
            res  != NULL &&
            size != NULL
        );

        const void* addr = (res + offset);

        void* mem = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
        assert(mem != NULL);

        return(mem);
    }

    IFB_WIN32_API_FUNC void
    pfm_memory_decommit(
        const void* cmt, const u32 size) {

        assert(cmt != NULL && size != 0);

        const bool did_free = VirtualFree(cmt, size, MEM_DECOMMIT);
        
        assert(did_free);
    }
};