#ifndef GUI_HPP
#define GUI_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------


    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct gui;
    struct gui_strings;
    struct gui_memory;
    struct gui_window_flags;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // context
    IFB_INTERNAL u32   gui_memory_requirement (void);
    IFB_INTERNAL gui*  gui_memory_init        (memory& stack_memory);
    IFB_INTERNAL void  gui_startup            (gui* g, memory& reserved_memory);
    IFB_INTERNAL void  gui_open               (gui* g);
    IFB_INTERNAL void  gui_close              (gui* g);
    IFB_INTERNAL void  gui_render             (gui* g);
    IFB_INTERNAL void  gui_validate           (gui* g);
    IFB_INTERNAL void  gui_main_window        (gui* g);

    // memory
    IFB_INTERNAL void* gui_memory_stack_alloc (gui* g, const u32 size);
    IFB_INTERNAL void  gui_memory_stack_free  (gui* g, void* mem);

    // engine
    IFB_INTERNAL void  gui_engine_imgui_demo  (gui* g);
    IFB_INTERNAL void  gui_engine_system      (gui* g);

    // renderer
    IFB_INTERNAL void  gui_renderer_camera    (gui* g);

    //--------------------------------------------------------------------
    // ENUMS
    //--------------------------------------------------------------------

    enum gui_window_flag_e {
        gui_window_flag_e_none              = 0,
        gui_window_flag_e_engine_imgui_demo = bit_value(0),
        gui_window_flag_e_engine_system     = bit_value(1),
        gui_window_flag_e_renderer_camera   = bit_value(2),
    };

    struct gui_window_flags : flags {

        inline bool get_engine_system     (void) { return(this->test(gui_window_flag_e_engine_system));     }
        inline bool get_engine_imgui_demo (void) { return(this->test(gui_window_flag_e_engine_imgui_demo)); }
        inline bool get_renderer_camera   (void) { return(this->test(gui_window_flag_e_renderer_camera));   }
    
        inline void update_engine_system     (bool b) { if (b) this->mask_on(gui_window_flag_e_engine_system);     else this->mask_off(gui_window_flag_e_engine_system);     }
        inline void update_engine_imgui_demo (bool b) { if (b) this->mask_on(gui_window_flag_e_engine_imgui_demo); else this->mask_off(gui_window_flag_e_engine_imgui_demo); }
        inline void update_renderer_camera   (bool b) { if (b) this->mask_on(gui_window_flag_e_renderer_camera);   else this->mask_off(gui_window_flag_e_renderer_camera);   }
    };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

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

    struct gui {
        b8                        is_open;
        gui_window_flags          window_flags;
        gui_memory                res_mem;
        inline static gui_strings* str;
    };

    struct gui_strings {
        inline static const char* menu_engine                 = "Engine";
        inline static const char* menu_engine_item_imgui_demo = "ImGui Demo";
        inline static const char* menu_engine_item_system     = "System";
        inline static const char* menu_renderer               = "Renderer";
        inline static const char* menu_renderer_item_camera   = "Camera"; 
        inline static const char* window_renderer_camera      = "Renderer Camera";
    };

};

#endif //GUI_HPP