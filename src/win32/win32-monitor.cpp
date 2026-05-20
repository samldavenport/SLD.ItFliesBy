#pragma once

#include "win32.hpp"

namespace ifb {

    struct win32_monitor_enumerator {
        u32              index;
        pfm_monitor_info monitor_info;        
    };


    static BOOL CALLBACK
    win32_monitor_enum_callback_get_primary(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data)

    IFB_PLATFORM_API u32
    pfm_monitor_count(
        void) {

        const u32 count = GetSystemMetrics(SM_CMONITORS);
        return(count);
    }
    
    IFB_PLATFORM_API u32
    pfm_monitor_primary_index(
        void) {

        return(0);
    }

    IFB_PLATFORM_API void
    pfm_monitor_get_working_area(
        pfm_monitor_working_area& working_area) {

        working_area.virtual_pixel_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        working_area.virtual_pixel_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }


    IFB_PLATFORM_API void
    pfm_monitor_get_info(
        const u32         mntr_index,
        pfm_monitor_info* mntr_info) {

    }

    // IFB_PLATFORM_API u32
    // win32_monitor_get_primary(
    //     void) {

    //     POINT        origin    = { 0, 0 };
    //     HMONITOR     h_monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
    //     os_monitor_handle monitor   = { h_monitor };
    //     return(monitor);
    // }

    // SLD_API_OS_FUNC os_monitor_handle
    // win32_monitor_from_point(
    //     const u32 x,
    //     const u32 y) {

    //     LONG     long_x    = *(LONG*)&x;
    //     LONG     long_y    = *(LONG*)&y;
    //     POINT    point     = { long_x, long_y };
    //     HMONITOR h_monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);

    //     os_monitor_handle monitor = { h_monitor };
    //     return(monitor);
    // }

    static BOOL CALLBACK
    win32_monitor_enum_callback(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data){

        constexpr u32 info_size = sizeof(MONITORINFOEX);
        constexpr u32 name_size = sizeof(MONITORINFOEX::szDevice);

        // cast the data and check if we should enumerate
        win32_monitor_enumerator* enumerator = (win32_monitor_enumerator*)dw_data;
        const bool should_enumerate = (
            enumerator != NULL && 
            enumerator->index < enumerator->count
        );

        MONITORINFOEX win32_monitor_info;
        win32_monitor_info.cbSize = info_size;

        if (should_enumerate) {

            // get the info for the current monitor handle
            GetMonitorInfo(h_monitor, (MONITORINFO*)&win32_monitor_info);

            // update the index
            ++enumerator->index;
        }        
        
        return(should_enumerate);
    }

};