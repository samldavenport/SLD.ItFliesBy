#pragma once

#include <Windows.h>
#include "ifb-engine.hpp"
#include "ifb-entity.hpp"
#include "ifb-game.hpp"
#include "ifb-input.hpp"
#include "ifb-types.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"

#include "collections.cpp"
#include "game-context.cpp"
#include "game-player-rig.cpp"
#include "gui.cpp"
#include "imgui.h"

using namespace ifb;

static const u32 SIZE_STACK       = size_kilobytes(64);
static const u32 SIZE_RESERVATION = size_megabytes(64);

static byte _stack_mem[SIZE_STACK];

static game_context* game_ctx;

bool game_proc    (eng_game_context* ctx);
void render_proc  (void);
void mem_map_init (eng_mem_map& mem_map);

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
    eng_context* ctx = eng_context_create(
        &mem_map,
        game_proc,
        render_proc
    );

    // engine startup 
    eng_context_startup();

    // create the game context
    game_ctx = game_context_create_and_init();
    assert(game_ctx);

    // open the gui
    ImGuiContext* imgui = eng_context_get_imgui();
    ImGui::SetCurrentContext(imgui);
    gui_open();

    // run the engine
    bool running = true;
    while (running) {
        running &= eng_context_run();
    }

    return(0);
}

static bool
game_proc(
    eng_game_context* ctx) {

    game_context_update_and_render(game_ctx);

    return(true);
}

static void 
render_proc(
    void) {

    gui_render();
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
    mem_map.tiles.size      = SIZE_RESERVATION;
    mem_map.tiles.ptr       = VirtualAlloc(NULL, SIZE_RESERVATION, MEM_RESERVE, PAGE_READONLY);

    assert(
        mem_map.stack.ptr      != NULL &&        
        mem_map.core.ptr       != NULL &&        
        mem_map.files.ptr      != NULL &&        
        mem_map.rendering.ptr  != NULL &&
        mem_map.entities.ptr   != NULL &&
        mem_map.gui.ptr        != NULL &&
        mem_map.components.ptr != NULL &&
        mem_map.quads.ptr      != NULL &&
        mem_map.physics.ptr    != NULL &&
        mem_map.tiles.ptr      != NULL
    );
}
