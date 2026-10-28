#ifndef ENG_PLATFORM_HPP
#define ENG_PLATFORM_HPP

#include "ifb-platform.hpp"

#define IFB_PLATFORM_API

namespace ifb {

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
};

#endif //ENG_PLATFORM_HPP
