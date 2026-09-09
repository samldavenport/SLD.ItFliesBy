#pragma once

#include <ifb-platform.hpp>
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

        void* addr = ((byte*)res + offset);

        void* mem = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
        assert(mem != NULL);

        return(mem);
    }

    IFB_WIN32_API_FUNC void
    pfm_memory_decommit(
        const void* cmt, const u32 size) {

        assert(cmt != NULL && size != 0);

        const bool did_free = VirtualFree((void*)cmt, size, MEM_DECOMMIT);
        
        assert(did_free);
    }
    
    IFB_PLATFORM_API void
    pfm_memory_get_info(
        pfm_memory_info& mem_info) {

        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
    
        mem_info.size_page        = sys_info.dwPageSize;
        mem_info.size_granularity = sys_info.dwAllocationGranularity;
    }
};
