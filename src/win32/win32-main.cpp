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
    mem_map.stack.size      = SIZE_STACK;
    mem_map.stack.ptr       = _stack_mem;
    mem_map.core.size       = SIZE_RESERVATION; 
    mem_map.core.ptr        = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.files.size      = SIZE_RESERVATION; 
    mem_map.files.ptr       = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.rendering.size  = SIZE_RESERVATION; 
    mem_map.rendering.ptr   = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.entities.size   = SIZE_RESERVATION; 
    mem_map.entities.ptr    = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.gui.size        = SIZE_RESERVATION; 
    mem_map.gui.ptr         = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.arenas.size     = SIZE_RESERVATION; 
    mem_map.arenas.ptr      = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.components.size = SIZE_RESERVATION;
    mem_map.components.ptr  = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.quads.size = SIZE_RESERVATION;
    mem_map.quads.ptr  = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);

    assert(
        mem_map.stack.ptr      != NULL &&        
        mem_map.core.ptr       != NULL &&        
        mem_map.files.ptr      != NULL &&        
        mem_map.rendering.ptr  != NULL &&
        mem_map.entities.ptr   != NULL &&
        mem_map.gui.ptr        != NULL &&
        mem_map.components.ptr != NULL &&
        mem_map.quads.ptr      != NULL
    );

    eng_context* ctx = eng_context_create(&mem_map);

    eng_context_startup();
    eng_gui_open();
    eng_context_run();

    return(0);
}