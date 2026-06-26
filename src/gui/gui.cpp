#pragma once

#include "gui.hpp"
#include "gui-engine.cpp"
#include "gui-renderer-camera.cpp"
#include "gui-controls.cpp"

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

        _gui = (gui*)stack_memory.ptr;
        _gui->is_open      = false;
        _gui->str          = NULL;
        _gui->window_flags.val = 0;
        return(_gui);
    }

    IFB_INTERNAL void
    gui_startup(
        memory& reserved_memory) {

        assert(
            _gui                    != NULL &&
            reserved_memory.address != 0 &&
            reserved_memory.size    != 0
        );

        const u32 size_strings = sizeof(gui_strings);
        const u32 size_stack   = (reserved_memory.size - size_strings);

        _gui->res_mem.sys_mem    = reserved_memory;
        _gui->str                = (gui_strings*)pfm_memory_commit (_gui->res_mem.sys_mem.ptr, 0,            size_strings);
        _gui->res_mem.stack.vptr =               pfm_memory_commit (_gui->res_mem.sys_mem.ptr, size_strings, size_stack);
        _gui->res_mem.stack.size = size_stack;
        _gui->res_mem.stack.pos  = 0;

        assert(
            _gui->res_mem.sys_mem.address != 0 &&
            _gui->res_mem.sys_mem.size    != 0 &&
            _gui->str                     != 0 && 
            _gui->res_mem.stack.address   != 0 && 
            _gui->res_mem.stack.size      != 0
        );
    }

    IFB_INTERNAL void
    gui_open(
        void) {

        gui_validate();
        _gui->is_open = true;
    }

    IFB_INTERNAL void
    gui_close(
        void) {

        gui_validate();
        _gui->is_open = false;
    }

    IFB_INTERNAL void
    gui_render(
        void) {

        gui_validate();

        if (!_gui->is_open) {
            return;
        }

        gui_main_window       ();
        gui_engine_imgui_demo ();
        gui_renderer_camera   ();
    }

    IFB_INTERNAL void
    gui_validate(
        void) {

        assert(
            _gui                        != NULL &&
            _gui->str                   != NULL &&
            _gui->res_mem.stack.address != 0    &&
            _gui->res_mem.stack.size    != 0    &&
            _gui->res_mem.stack.pos     <= _gui->res_mem.stack.size
        );
    }

    IFB_INTERNAL void
    gui_main_window(
        void) {
        
        gui_validate();

        static bool is_selected = false;
        const auto str          = _gui->str;

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu(str->menu_engine)) {

                is_selected = _gui->window_flags.get_engine_imgui_demo();
                ImGui::MenuItem(str->menu_engine_item_imgui_demo, NULL, &is_selected);
                _gui->window_flags.update_engine_imgui_demo(is_selected);

                is_selected = _gui->window_flags.get_engine_system();
                ImGui::MenuItem(str->menu_engine_item_system, NULL, &is_selected);
                _gui->window_flags.update_engine_system(is_selected);
            
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(str->menu_renderer)) {

                is_selected = _gui->window_flags.get_renderer_camera();
                ImGui::MenuItem(str->menu_renderer_item_camera, NULL, &is_selected);
                _gui->window_flags.update_renderer_camera(is_selected);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
};