#ifndef IFB_ENGINE_HPP
#define IFB_ENGINE_HPP

#include "sld.hpp"
#include "ifb-platform.hpp"
#include "ifb-input.hpp"

#ifdef IFB_ENGINE_STATIC
#   define IFB_ENGINE_API
#elif defined(IFB_ENGINE_DLL_EXPORT)
#   define IFB_ENGINE_API __declspec(dllexport)
#else
#   define IFB_ENGINE_API __declspec(dllimport)
#endif

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------

    using eng_file_handle = u32;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct eng_system_info;
    struct eng_context;
    struct eng_mem;
    struct eng_mem_map;

    //--------------------------------------------------------------------
    // CONTEXT
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context* eng_context_create                       (const eng_mem_map* mem_map);
    IFB_ENGINE_API void         eng_context_startup                      (void);
    IFB_ENGINE_API void         eng_context_run                          (void);
    IFB_ENGINE_API void         eng_context_shutdown                     (void);
    IFB_ENGINE_API void         eng_context_destroy                      (eng_context* ctx);    

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_window_open                                  (void);
    IFB_ENGINE_API void eng_window_did_close                             (void);
    IFB_ENGINE_API void eng_window_set_pos                               (const u32 pos_x, const u32 window_pos_y);
    IFB_ENGINE_API void eng_window_set_size                              (const u32 width, const u32 height);
    
    //--------------------------------------------------------------------
    // INPUT
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_input_set_key_down                           (const input_keycode key_ptr);
    IFB_ENGINE_API void eng_input_set_key_up                             (const input_keycode key_ptr);
    IFB_ENGINE_API void eng_input_mouse_click_left_down                  (void);
    IFB_ENGINE_API void eng_input_mouse_click_left_up                    (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_down                 (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_up                   (void);
    IFB_ENGINE_API void eng_input_mouse_move                             (const u32 x, const u32 y);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API eng_file_handle eng_file_ro_create_new                (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_ro_open_existing             (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_ro_open_always               (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_ro_overwrite                 (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_wo_create_new                (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_wo_open_existing             (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_wo_open_always               (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_wo_overwrite                 (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_rw_create_new                (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_rw_open_existing             (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_rw_open_always               (const cchar8* path);
    IFB_ENGINE_API eng_file_handle eng_file_rw_overwrite                 (const cchar8* path);
    IFB_ENGINE_API void            eng_file_close                        (const eng_file_handle hnd);
    IFB_ENGINE_API u32             eng_file_get_size                     (const eng_file_handle hnd);
    IFB_ENGINE_API void            eng_file_set_cursor                   (const eng_file_handle hnd, const u32 cursor);
    IFB_ENGINE_API u32             eng_file_read                         (const eng_file_handle hnd, const u32 buffer_size, byte* buffer_ptr);
    IFB_ENGINE_API u32             eng_file_write                        (const eng_file_handle hnd, const u32 buffer_size, byte* buffer_ptr);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct eng_system_info {
        struct {
            u32              count;
            pfm_monitor_info primary;
            pfm_monitor_area working_area;
        } monitor;
    };

    struct eng_mem {
        void* ptr;
        u64   size;
    };

    struct eng_mem_map {
        eng_mem stack;
        eng_mem files;
        eng_mem core;
        eng_mem rendering;
    };
}
#endif  //IFB_ENGINE_HPP