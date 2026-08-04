#pragma once

#include <Windows.h>
#include "ifb-engine.hpp"
#include "ifb-entity.hpp"
#include "ifb.hpp"

using namespace ifb;

static const u32 SIZE_STACK       = size_kilobytes(64);
static const u32 SIZE_RESERVATION = size_megabytes(64);

static byte _stack_mem[SIZE_STACK];

static entity_id q_id_0;
static entity_id q_id_1;
static entity_id q_id_2;

bool game_proc (eng_game_context* ctx);
void mem_map_init  (eng_mem_map& mem_map);

int WINAPI
wWinMain(
    HINSTANCE h_instance,
    HINSTANCE h_prevInstance,
    PWSTR     p_cmd_line,
    int       n_cmd_show) {

    // initialize the memory map
    eng_mem_map mem_map;
    mem_map_init(mem_map);

    // create the engine context
    eng_context* ctx = eng_context_create(&mem_map, game_proc);

    // engine startup 
    eng_context_startup();
    eng_gui_open();
    
    // create test quad 
    q_id_0 = eng_entity_create("HELLO-QUAD-1");
    
    eng_entity_add_components(q_id_0, ENTITY_ARCHETYPE_QUAD.val);

    quad q_0 = {0};
    q_0.color.hex         = 0xFF0000FF;
    q_0.dimensions.width  = 0.2;
    q_0.dimensions.height = 0.2;
    q_0.position          = {0};

    eng_entity_update_quad(q_id_0, q_0);

    // run the engine
    eng_context_run();

    return(0);
}

static bool
game_proc(
    eng_game_context* ctx) {

    assert(ctx);

    eng_entity_render(q_id_0);

    return(true);
}

inline void
mem_map_init(
    eng_mem_map& mem_map) {
    
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
    mem_map.quads.size      = SIZE_RESERVATION;
    mem_map.quads.ptr       = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);
    mem_map.physics.size    = SIZE_RESERVATION;
    mem_map.physics.ptr     = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);

    assert(
        mem_map.stack.ptr      != NULL &&        
        mem_map.core.ptr       != NULL &&        
        mem_map.files.ptr      != NULL &&        
        mem_map.rendering.ptr  != NULL &&
        mem_map.entities.ptr   != NULL &&
        mem_map.gui.ptr        != NULL &&
        mem_map.components.ptr != NULL &&
        mem_map.quads.ptr      != NULL &&
        mem_map.physics.ptr    != NULL
    );
}
