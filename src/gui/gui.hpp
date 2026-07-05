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
    IFB_INTERNAL void  gui_open               (void);
    IFB_INTERNAL void  gui_close              (void);
    IFB_INTERNAL void  gui_render             (void);
    IFB_INTERNAL void  gui_main_window        (void);

    // engine
    IFB_INTERNAL void  gui_engine_imgui_demo  (bool& is_open);
    IFB_INTERNAL void  gui_engine_system      (bool& is_open);

    // renderer
    IFB_INTERNAL void  gui_renderer_camera    (bool& is_open);

    // controls
    IFB_INTERNAL void  gui_control_vec3        (const char* name, vec3& v);

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

    struct gui_strings {
        inline static const char* menu_engine                 = "Engine";
        inline static const char* menu_engine_item_imgui_demo = "ImGui Demo";
        inline static const char* menu_engine_item_system     = "System";
        inline static const char* menu_renderer               = "Renderer";
        inline static const char* menu_renderer_item_camera   = "Camera"; 
        inline static const char* window_renderer_camera      = "Renderer Camera";
        inline static const char* camera_origin               = "Camera Origin";
        inline static const char* camera_target               = "Camera Target";
    } static * _gui_str;

};

#endif //GUI_HPP