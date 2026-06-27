#ifndef GUI_HPP
#define GUI_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr f32 GUI_DRAG_FLOAT_SPEED =  0.001f;
    static constexpr f32 GUI_DRAG_FLOAT_MIN   = -1.000f;
    static constexpr f32 GUI_DRAG_FLOAT_MAX   =  1.000f;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct gui;
    struct gui_strings;
    struct gui_memory;
    struct gui_window_flags;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static gui* _gui;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // context
    IFB_INTERNAL u32   gui_memory_requirement (void);
    IFB_INTERNAL gui*  gui_memory_init        (memory& stack_memory);
    IFB_INTERNAL void  gui_startup            (memory& reserved_memory);
    IFB_INTERNAL void  gui_open               (void);
    IFB_INTERNAL void  gui_close              (void);
    IFB_INTERNAL void  gui_render             (void);
    IFB_INTERNAL void  gui_validate           (void);
    IFB_INTERNAL void  gui_main_window        (void);

    // memory
    IFB_INTERNAL void* gui_memory_stack_alloc (const u32 size);
    IFB_INTERNAL void  gui_memory_stack_free  (void* mem);

    // engine
    IFB_INTERNAL void  gui_engine_imgui_demo  (void);
    IFB_INTERNAL void  gui_engine_system      (void);

    // renderer
    IFB_INTERNAL void  gui_renderer_camera    (void);

    // controls
    IFB_INTERNAL void  gui_control_vec3        (const char* name, vec3& v);

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
        inline static const char* camera_origin               = "Camera Origin";
        inline static const char* camera_target               = "Camera Target";
    };

};

#endif //GUI_HPP