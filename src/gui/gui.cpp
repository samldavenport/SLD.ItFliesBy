#pragma once

#include "gui.hpp"

namespace ifb {

    IFB_INTERNAL u32
    gui_memory_requirement(
        void) {

        const u32 mem_req = sizeof(gui);
        return(mem_req);
    }

    IFB_INTERNAL gui*
    gui_memory_init(
        memory& stack_memory) {

        const u32 mem_req = gui_memory_requirement();
        assert(
            stack_memory.ptr  != NULL &&
            stack_memory.size == mem_req
        );

        auto g = (gui*)stack_memory.ptr;
        g->is_open      = false;
        g->str          = NULL;
        g->window_flags.val = 0;
        return(g);
    }

    IFB_INTERNAL void
    gui_startup(
        gui*    g,
        memory& reserved_memory) {

        assert(
            g                       != NULL &&
            reserved_memory.address != 0 &&
            reserved_memory.size    != 0
        );

        const u32 size_strings = sizeof(gui_strings);
        const u32 size_stack   = (reserved_memory.size - size_strings);

        g->res_mem.sys_mem    = reserved_memory;
        g->str                = (gui_strings*)pfm_memory_commit (g->res_mem.sys_mem.ptr, 0,            size_strings);
        g->res_mem.stack.vptr =               pfm_memory_commit (g->res_mem.sys_mem.ptr, size_strings, size_stack);
        g->res_mem.stack.size = size_stack;
        g->res_mem.stack.pos  = 0;

        assert(
            g->res_mem.sys_mem.address != 0 &&
            g->res_mem.sys_mem.size    != 0 &&
            g->str                     != 0 && 
            g->res_mem.stack.address   != 0 && 
            g->res_mem.stack.size      != 0
        );
    }

    IFB_INTERNAL void
    gui_open(
        gui* g) {

        gui_validate(g);
        g->is_open = true;
    }

    IFB_INTERNAL void
    gui_close(
        gui* g) {

        gui_validate(g);
        g->is_open = false;
    }

    IFB_INTERNAL void
    gui_render(
        gui* g) {

        gui_validate(g);

        

        if (!g->is_open) {
            return;
        }

        gui_main_window(g);
    }

    IFB_INTERNAL void
    gui_validate(
        gui* g) {

        assert(
            g                        != NULL &&
            g->str                   != NULL &&
            g->res_mem.stack.address != 0    &&
            g->res_mem.stack.size    != 0    &&
            g->res_mem.stack.pos     <= g->res_mem.stack.size
        );
    }

    IFB_INTERNAL void
    gui_main_window(
        gui* g) {
        
        gui_validate(g);

        static bool is_selected = false;
        const auto str          = g->str;

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu(str->menu_engine)) {

                is_selected = g->window_flags.get_engine_system();
                ImGui::MenuItem(str->menu_engine_item_system, NULL, &is_selected);
                g->window_flags.update_engine_system(is_selected);
            
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(str->menu_renderer)) {

                is_selected = g->window_flags.get_renderer_camera();
                ImGui::MenuItem(str->menu_renderer_item_camera, NULL, &is_selected);
                g->window_flags.update_renderer_camera(is_selected);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
};