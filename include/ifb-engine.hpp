#ifndef IFB_ENGINE_HPP
#define IFB_ENGINE_HPP

#include "sld.hpp"
#include "ifb-platform.hpp"

using namespace sld;

#define IFB_ENGINE_API

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct eng_context;
    struct eng_mem;

    //--------------------------------------------------------------------
    // CONTEXT
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context* eng_context_create  (const eng_mem_map* mem_map);
    IFB_ENGINE_API void         eng_context_destroy (eng_context* ctx);    

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_window_open                   (void);
    IFB_ENGINE_API void eng_window_did_close              (void);
    IFB_ENGINE_API void eng_window_set_pos                (const u32 pos_x, const u32 window_pos_y);
    IFB_ENGINE_API void eng_window_set_size               (const u32 width, const u32 height);
    IFB_ENGINE_API void eng_window_set_keys_down          (const u32 key_count, const pfm_window_key_code* key_ptr)
    IFB_ENGINE_API void eng_window_set_keys_up            (const u32 key_count, const pfm_window_key_code* key_ptr)
    IFB_ENGINE_API void eng_window_mouse_click_left_down  (void);
    IFB_ENGINE_API void eng_window_mouse_click_left_up    (void);
    IFB_ENGINE_API void eng_window_mouse_click_right_down (void);
    IFB_ENGINE_API void eng_window_mouse_click_right_up   (void);
    IFB_ENGINE_API void eng_window_mouse_move             (const u32 x, const u32 y);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_file_read_complete  (const pfm_file_handle file);
    IFB_ENGINE_API void eng_file_write_complete (const pfm_file_handle file);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

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