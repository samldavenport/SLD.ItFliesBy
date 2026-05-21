#pragma once

#include "win32.hpp"

namespace ifb {

    struct win32_monitor_enumerator {
        u32               index;
        u32               requested;
        HMONITOR          handle;
        pfm_monitor_info* info;
    };

    static BOOL CALLBACK
    win32_monitor_enum_callback_get_primary(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data);

    static BOOL CALLBACK
    win32_monitor_enum_callback_get_info(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data);

    IFB_PLATFORM_API u32
    pfm_monitor_count(
        void) {

        const u32 count = GetSystemMetrics(SM_CMONITORS);
        return(count);
    }
    
    IFB_PLATFORM_API u32
    win32_monitor_get_primary_index(
        void) {

        win32_monitor_enumerator enumerator;
        enumerator.index = 0;

        (void)EnumDisplayMonitors(
            NULL,
            NULL,
            win32_monitor_enum_callback_get_primary,
            (LPARAM)&enumerator
        );

        return(enumerator.index);
    }

    IFB_PLATFORM_API void
    pfm_monitor_get_working_area(
        pfm_monitor_area& working_area) {

        working_area.virtual_pixel_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        working_area.virtual_pixel_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }

    IFB_PLATFORM_API void
    pfm_monitor_get_info(
        const u32         mntr_index,
        pfm_monitor_info* mntr_info) {

        const u32 count = GetSystemMetrics(SM_CMONITORS);
        assert(mntr_index < count);

        win32_monitor_enumerator enumerator;
        enumerator.index     = 0;
        enumerator.requested = mntr_index;
        enumerator.info      = mntr_info; 

        (void)EnumDisplayMonitors(
            NULL,
            NULL,
            win32_monitor_enum_callback_get_info,
            (LPARAM)&enumerator
        );

    }

    IFB_WIN32_INTERNAL BOOL CALLBACK
    win32_monitor_enum_callback_get_primary(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data){

        constexpr u32 info_size = sizeof(MONITORINFOEX);

        win32_monitor_enumerator* enumerator = (win32_monitor_enumerator*)dw_data;

        MONITORINFOEX win32_monitor_info;
        win32_monitor_info.cbSize = info_size;

        // get the info for the current monitor handle
        GetMonitorInfo(h_monitor, (MONITORINFO*)&win32_monitor_info);

        const bool found = (win32_monitor_info.rcMonitor.top == 0 && win32_monitor_info.rcMonitor.left == 0);         
        return(!found);
    }

    IFB_WIN32_INTERNAL BOOL CALLBACK
    win32_monitor_enum_callback_get_info(
        HMONITOR h_monitor,
        HDC      hdc_monitor,
        LPRECT   lprc_monitor,
        LPARAM   dw_data) {

        constexpr u32 info_size = sizeof(MONITORINFOEX);
        constexpr u32 name_size = sizeof(MONITORINFOEX::szDevice);

        win32_monitor_enumerator* enumerator = (win32_monitor_enumerator*)dw_data;
        if (enumerator->index != enumerator->requested) {
            ++enumerator->index;
            return(true);
        }
        
        MONITORINFOEX win32_monitor_info;
        win32_monitor_info.cbSize = info_size;
        GetMonitorInfo(h_monitor, (MONITORINFO*)&win32_monitor_info);

        DEVMODE dev_mode = {};
        dev_mode.dmSize  = sizeof(DEVMODE);
        const bool has_settings = EnumDisplaySettings(
            win32_monitor_info.szDevice,
            ENUM_CURRENT_SETTINGS,
            &dev_mode
        );

        auto info = enumerator->info;
        info->pixel_width     = (win32_monitor_info.rcMonitor.right  - win32_monitor_info.rcMonitor.left);        
        info->pixel_height    = (win32_monitor_info.rcMonitor.bottom - win32_monitor_info.rcMonitor.top);        
        info->x               = win32_monitor_info.rcMonitor.left;        
        info->y               = win32_monitor_info.rcMonitor.top;
        info->refresh_rate_hz = (has_settings) ? dev_mode.dmDisplayFrequency : 0; 

        return(false);
    }
};