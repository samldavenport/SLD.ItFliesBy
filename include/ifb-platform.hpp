#ifndef IFB_PLATFORM_HPP
#define IFB_PLATFORM_HPP

#include "sld.hpp"

#define IFB_PLATFORM_API

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // PRIMITIVE TYPES
    //--------------------------------------------------------------------

    typedef u32   pfm_window_event_flags;
    typedef u32   pfm_window_key_code;
    typedef void* pfm_file_handle;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct pfm_window_context;
    struct pfm_window_config;
    struct pfm_file_buffer;
    struct pfm_monitor_info;
    struct pfm_monitor_area;

    //--------------------------------------------------------------------
    // ENUMS
    //--------------------------------------------------------------------

    enum window_event_flag_e {
        window_event_flag_e_none              = 0,
        window_event_flag_e_close             = bit_value(1),
        window_event_flag_e_minimize          = bit_value(2),
        window_event_flag_e_maximize          = bit_value(3),
        window_event_flag_e_position_update   = bit_value(4),
        window_event_flag_e_size_update       = bit_value(5),
        window_event_flag_e_key_down          = bit_value(6),
        window_event_flag_e_key_up            = bit_value(7),
        window_event_flag_e_mouse_click_left  = bit_value(8),
        window_event_flag_e_mouse_click_right = bit_value(9),
        window_event_flag_e_mouse_move        = bit_value(10),
        window_event_flag_e_mouse_scroll      = bit_value(11)
    };

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_PLATFORM_API void  pfm_window_open           (const pfm_window_config* cfg);
    IFB_PLATFORM_API void  pfm_window_process_events (void);
    IFB_PLATFORM_API void  pfm_window_close          (void);

    //--------------------------------------------------------------------
    // MONITOR
    //--------------------------------------------------------------------

    IFB_PLATFORM_API u32   pfm_monitor_count             (void);
    IFB_PLATFORM_API u32   pfm_monitor_get_primary_index (void);
    IFB_PLATFORM_API void  pfm_monitor_get_working_area  (pfm_monitor_area& working_area);
    IFB_PLATFORM_API void  pfm_monitor_get_info          (const u32 mntr_index, pfm_monitor_info* mntr_info); 

    //--------------------------------------------------------------------
    // MEMORY
    //--------------------------------------------------------------------

    IFB_PLATFORM_API void* pfm_memory_commit    (const void* res, const u32 offset, const u32 size);
    IFB_PLATFORM_API void  pfm_memory_decommit  (const void* cmt);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------

    IFB_PLATFORM_API pfm_file_handle pfm_file_open_r        (const cchar8*  path);
    IFB_PLATFORM_API pfm_file_handle pfm_file_open_w        (const cchar8*  path);
    IFB_PLATFORM_API pfm_file_handle pfm_file_open_rw       (const cchar8*  path);
    IFB_PLATFORM_API pfm_file_handle pfm_file_open_async_r  (const cchar8*  path);
    IFB_PLATFORM_API pfm_file_handle pfm_file_open_async_w  (const cchar8*  path);
    IFB_PLATFORM_API pfm_file_handle pfm_file_open_async_rw (const cchar8*  path);
    IFB_PLATFORM_API u32             pfm_file_size          (const pfm_file_handle file);
    IFB_PLATFORM_API bool            pfm_file_close         (const pfm_file_handle file);
    IFB_PLATFORM_API u32             pfm_file_read          (const pfm_file_handle file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32             pfm_file_write         (const pfm_file_handle file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32             pfm_file_async_read    (const pfm_file_handle file, pfm_file_buffer* buffer);
    IFB_PLATFORM_API u32             pfm_file_async_write   (const pfm_file_handle file, pfm_file_buffer* buffer);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct pfm_window_config {
        cchar8* title;
        struct  {
            u32 x;
            u32 y;
            u32 width;
            u32 height;
        } init_dims;
    };

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
        struct {
            u32                 key_count_up;
            u32                 key_count_down;
            pfm_window_key_code key_codes_up   [IFB_CONFIG_WINDOW_MAX_KEYS];
            pfm_window_key_code key_codes_down [IFB_CONFIG_WINDOW_MAX_KEYS];
        } keys;
    };

    struct pfm_file_buffer {
        byte* data;
        u32   size;
        u32   length;
        u32   offset;
        u32   cursor;
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
};

#endif //IFB_PLATFORM_HPP