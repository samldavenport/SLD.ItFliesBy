#ifndef IFB_PLATFORM_HPP
#define IFB_PLATFORM_HPP

#include "ifb.hpp"
#include "sld.hpp"
#include "sld-opengl.hpp"

#define IFB_PLATFORM_API

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------

    using pfm_file_mode          = u32;
    using pfm_file_access_flags  = u32;
    using pfm_file_share_flags   = u32;
    using pfm_hnd_file           = void*;
    using pfm_window_event_flags = u32;
    using pfm_hnd_thread         = void*;
    using pfm_fptr_thread_proc   = void (*) (void* ctx); 
    
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct pfm_context;
    struct pfm_window_context;
    struct pfm_window_config;
    struct pfm_file_config;
    struct pfm_file_buffer;
    struct pfm_file_dialog;
    struct pfm_monitor_info;
    struct pfm_monitor_area;
    struct pfm_memory_info;
    struct pfm_thread_mutex;
    struct pfm_thread_cond;
    struct pfm_thread;
    
    //--------------------------------------------------------------------
    // ENUMS
    //--------------------------------------------------------------------

    enum pfm_window_event_flag_e {
        pfm_window_event_flag_e_none              = 0,
        pfm_window_event_flag_e_close             = bit_value(1),
        pfm_window_event_flag_e_minimize          = bit_value(2),
        pfm_window_event_flag_e_maximize          = bit_value(3),
        pfm_window_event_flag_e_position_update   = bit_value(4),
        pfm_window_event_flag_e_size_update       = bit_value(5),
        pfm_window_event_flag_e_key_down          = bit_value(6),
        pfm_window_event_flag_e_key_up            = bit_value(7),
        pfm_window_event_flag_e_mouse_click_left  = bit_value(8),
        pfm_window_event_flag_e_mouse_click_right = bit_value(9),
        pfm_window_event_flag_e_mouse_move        = bit_value(10),
        pfm_window_event_flag_e_mouse_scroll      = bit_value(11)
    };

    enum pfm_file_access_flag_e {
        pfm_file_access_flag_e_none               = 0,        
        pfm_file_access_flag_e_read               = bit_value(0),        
        pfm_file_access_flag_e_write              = bit_value(1)
    };

    enum pfm_file_share_flag_e {
        pfm_file_share_flag_e_none                = 0,
        pfm_file_share_flag_e_read                = bit_value(0),
        pfm_file_share_flag_e_write               = bit_value(1),
        pfm_file_share_flag_e_delete              = bit_value(2)
    };

    enum pfm_file_mode_e {
        pfm_file_mode_e_create_new                = 0, 
        pfm_file_mode_e_open_existing             = 1, 
        pfm_file_mode_e_open_always               = 2, 
        pfm_file_mode_e_overwrite_existing        = 3 
    };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct pfm_window_context {
        pfm_window_event_flags event_flags;
        struct {
            u32 width;
            u32 height;
            u32 x;
            u32 y;
        } dims;
        struct {
            u32 x;
            u32 y;
            f32 scroll;
        } mouse;
    };
    
    struct pfm_window_config {
        cchar* title;
        struct  {
            u32 x;
            u32 y;
            u32 width;
            u32 height;
        } init_dims;
    };

    struct pfm_file_config {
        const cchar*          path;
        pfm_file_mode         mode;
        pfm_file_access_flags access_flags;
        pfm_file_share_flags  share_flags;
        bool                  is_async;
    };

    struct pfm_file_buffer {
        byte* data;
        u32   size;
        u32   length;
        u32   offset;
        u32   cursor;
    };

    struct pfm_file_dialog {
        const cchar* path;
        const cchar* filter_type; 
        const cchar* filter_desc; 
    };

    struct pfm_monitor_info {
        u32 index;
        u32 refresh_rate_hz;
        u32 pixel_width;
        u32 pixel_height;
        u32 x;
        u32 y;
    };

    struct pfm_monitor_area {
        u32 virtual_pixel_width;
        u32 virtual_pixel_height;
    };

    struct pfm_memory_info {
        u32 size_page;
        u32 size_granularity;
    };

    struct pfm_thread {
        pfm_hnd_thread       hnd;
        pfm_fptr_thread_proc proc;
        u32                  index;
        void*                ctx;
    };

    struct pfm_thread_mutex { void* ctx; };
    struct pfm_thread_cond  { void* ctx; };

    //--------------------------------------------------------------------
    // context 
    //--------------------------------------------------------------------
    
    IFB_PLATFORM_API void pfm_context_startup  (void);
    IFB_PLATFORM_API void pfm_context_shutdown (void);

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_PLATFORM_API void  pfm_window_open               (const pfm_window_config* cfg);
    IFB_PLATFORM_API void  pfm_window_frame_start        (void);
    IFB_PLATFORM_API void  pfm_window_frame_render       (void);
    IFB_PLATFORM_API void  pfm_window_process_events     (void);
    IFB_PLATFORM_API void  pfm_window_close              (void);
    IFB_PLATFORM_API bool  pfm_window_quit_received      (void);

    //--------------------------------------------------------------------
    // MONITOR
    //--------------------------------------------------------------------

    IFB_PLATFORM_API u32   pfm_monitor_count             (void);
    IFB_PLATFORM_API u32   pfm_monitor_get_primary_index (void);
    IFB_PLATFORM_API void  pfm_monitor_get_working_area  (pfm_monitor_area& working_area);
    IFB_PLATFORM_API void  pfm_monitor_get_info          (const u32 mntr_index, pfm_monitor_info* mntr_info); 

    //--------------------------------------------------------------------
    // GRAPHICS
    //--------------------------------------------------------------------

    IFB_PLATFORM_API void  pfm_graphics_init_opengl      (gl_context* ctx);
    IFB_PLATFORM_API void  pfm_graphics_init_imgui       (void);

    //--------------------------------------------------------------------
    // MEMORY
    //--------------------------------------------------------------------

    IFB_PLATFORM_API void* pfm_memory_commit   (const void* res, const u32 offset, const u32 size);
    IFB_PLATFORM_API void  pfm_memory_decommit (const void* cmt, const u32 size);
    IFB_PLATFORM_API void  pfm_memory_get_info (pfm_memory_info& mem_info);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------

    IFB_PLATFORM_API pfm_hnd_file pfm_file_open          (const pfm_file_config* cfg);
    IFB_PLATFORM_API pfm_hnd_file pfm_file_open_async_r  (const cchar*  path);
    IFB_PLATFORM_API pfm_hnd_file pfm_file_open_async_w  (const cchar*  path);
    IFB_PLATFORM_API pfm_hnd_file pfm_file_open_async_rw (const cchar*  path);
    IFB_PLATFORM_API u32          pfm_file_size          (const pfm_hnd_file file);
    IFB_PLATFORM_API void         pfm_file_close         (const pfm_hnd_file file);
    IFB_PLATFORM_API u32          pfm_file_read          (const pfm_hnd_file file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32          pfm_file_write         (const pfm_hnd_file file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32          pfm_file_async_read    (const pfm_hnd_file file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32          pfm_file_async_write   (const pfm_hnd_file file, pfm_file_buffer* buffer);

    //--------------------------------------------------------------------
    // FILE DIALOG 
    //--------------------------------------------------------------------
   
    IFB_PLATFORM_API bool pfm_file_dialog_open (pfm_file_dialog& dlg);
    IFB_PLATFORM_API bool pfm_file_dialog_save (pfm_file_dialog& dlg);

    //--------------------------------------------------------------------
    // SYSTEM
    //--------------------------------------------------------------------
   
    IFB_PLATFORM_API u64 pfm_system_timestamp (void);
    IFB_PLATFORM_API u64 pfm_system_frequency (void);

    //--------------------------------------------------------------------
    // THREADS 
    //--------------------------------------------------------------------

    
};

#endif //IFB_PLATFORM_HPP
