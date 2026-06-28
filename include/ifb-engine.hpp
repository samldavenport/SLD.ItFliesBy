#ifndef IFB_ENGINE_HPP
#define IFB_ENGINE_HPP

#include "sld.hpp"
#include "ifb-types.hpp"
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
    using eng_entity_id   = u32;


    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct eng_system_info;
    struct eng_context;
    struct eng_mem;
    struct eng_mem_map;

    struct eng_arena_handle : handle { using handle::handle; };

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
    // GUI
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_gui_open  (void);
    IFB_ENGINE_API void eng_gui_close (void);
    
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
    // CAMERA
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_camera_get_origin (vec3& origin);
    IFB_ENGINE_API void eng_camera_get_target (vec3& target);
    IFB_ENGINE_API void eng_camera_set_origin (const vec3& origin);
    IFB_ENGINE_API void eng_camera_set_target (const vec3& target);

    //--------------------------------------------------------------------
    // ENTITIES
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_entity_id eng_entity_create         (const cchar8*       tag);
    IFB_ENGINE_API bool          eng_entity_destroy_by_id  (const eng_entity_id id);
    IFB_ENGINE_API bool          eng_entity_destroy_by_tag (const cchar8*       tag);
    IFB_ENGINE_API const cchar8* eng_entity_get_tag        (const eng_entity_id id);

    //--------------------------------------------------------------------
    // IMAGES
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32  image_size (const cchar8* path);
    IFB_ENGINE_API bool image_load (const cchar8* path);
    
    //--------------------------------------------------------------------
    // ARENAS
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_arena_handle eng_arena_alloc     (void);
    IFB_ENGINE_API void             eng_arena_free      (const eng_arena_handle arena);
    IFB_ENGINE_API void             eng_arena_reset     (const eng_arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_save      (const eng_arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_size_free (const eng_arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_size_used (const eng_arena_handle arena);
    IFB_ENGINE_API void             eng_arena_revert    (const eng_arena_handle arena, const u32 save);
    IFB_ENGINE_API void*            eng_arena_push      (const eng_arena_handle arena, const u32 size);

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
        eng_mem entities;
        eng_mem gui;
        eng_mem arenas;
    };
}
#endif  //IFB_ENGINE_HPP