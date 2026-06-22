#ifndef GUI_HPP
#define GUI_HPP

#include "ifb.hpp"

namespace ifb {

    struct gui;
    struct gui_strings;
    struct gui_memory;

    using gui_window_flags = u32;

    struct gui_memory {
        memory sys_mem;
        struct {
            union {
                void* vptr;
                addr  address;
            };
            u32  size;
            u32  pos;
        } stack;
    };

    struct gui_strings {
        inline static const char* menu_engine               = "Engine";
        inline static const char* menu_engine_item_system   = "System";
        inline static const char* menu_renderer             = "Renderer";
        inline static const char* menu_renderer_item_camera = "Camera"; 
    };


    struct gui {
        b8                        is_open;
        gui_window_flags          window_flags;
        gui_memory                res_mem;
        inline static gui_strings* str;
    };

    enum gui_window_flag_e {
        gui_window_flag_e_none            = 0,
        gui_window_flag_e_engine_system   = bit_value(0),
        gui_window_flag_e_renderer_camera = bit_value(1),
    };

    IFB_INTERNAL u32  gui_memory_requirement (void);
    IFB_INTERNAL gui* gui_memory_init        (memory& stack_memory);
    IFB_INTERNAL void gui_startup            (gui* g, memory& reserved_memory);
    IFB_INTERNAL void gui_open               (gui* g);
    IFB_INTERNAL void gui_close              (gui* g);
    IFB_INTERNAL void gui_render             (gui* g);
    IFB_INTERNAL void gui_validate           (gui* g);
    IFB_INTERNAL void gui_main_window        (gui* g);

    IFB_INTERNAL void* gui_memory_stack_alloc (gui* g, const u32 size);
    IFB_INTERNAL void  gui_memory_stack_free  (gui* g, void* mem);

};

#endif //GUI_HPP