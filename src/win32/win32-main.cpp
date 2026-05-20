#pragma once

#include <Windows.h>
#include "ifb.hpp"

using namespace ifb;

static const u32 SIZE_STACK       = size_kilobytes(64);
static const u32 SIZE_RESERVATION = size_megabytes(64);

static byte _stack_mem[SIZE_STACK];   


int WINAPI
wWinMain(
    HINSTANCE h_instance,
    HINSTANCE h_prevInstance,
    PWSTR     p_cmd_line,
    int       n_cmd_show) {

    eng_mem_map mem_map;
    mem_map.stack.ptr      = _stack_mem;
    mem_map.stack.size     = SIZE_STACK;
    mem_map.core.ptr       = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.core.size      = SIZE_RESERVATION; 
    mem_map.files.ptr      = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.files.size     = SIZE_RESERVATION; 
    mem_map.rendering.ptr  = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.rendering.size = SIZE_RESERVATION; 

    assert(
        mem_map.stack.ptr     != NULL &&        
        mem_map.core.ptr      != NULL &&        
        mem_map.files.ptr     != NULL &&        
        mem_map.rendering.ptr != NULL
    );

    eng_context* ctx = eng_context_create(&mem_map);

    return(0);
}