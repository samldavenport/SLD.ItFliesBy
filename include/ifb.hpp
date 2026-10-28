#ifndef IFB_HPP
#define IFB_HPP

#define STB_IMAGE_IMPLEMENTATION

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <stb_image.h>

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif

using namespace sld;

#define IFB_U32(type)     struct type : sld::strong_u32 { using strong_u32::strong_u32; };
#define IFB_U16(type)     struct type : sld::strong_u16 { using strong_u16::strong_u16; };
#define IFB_U8(type)      struct type : sld::strong_u8  { using strong_u8::strong_u8;   };
#define IFB_FLAGS32(type) struct type : flags           { using flags;                  };
#define IFB_HANDLE(hnd)   struct hnd  : handle          { using handle::handle;         };

#define IFB_INTERNAL static
#define IFB_INLINE   inline

#ifdef _WIN32 

//imgui win32 callback
extern IMGUI_IMPL_API LRESULT 
ImGui_ImplWin32_WndProcHandler(
    HWND   hWnd, 
    UINT   msg, 
    WPARAM wParam, 
    LPARAM lParam);

#endif

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 INVALID_U16       = 0xFFFF;
    constexpr u32 INVALID_U32       = 0xFFFFFFFF;
    constexpr u32 INVALID_INDEX     = INVALID_U32;
    constexpr u32 INVALID_HASH_32   = INVALID_U32;
    constexpr u32 INVALID_HANDLE    = INVALID_U32;
    constexpr u32 INVALID_ID        = INVALID_U32;
    constexpr f32 POS_MAX           =  1.0f;
    constexpr f32 POS_MIN           = -1.0f;
    
    struct text {
        cchar* cstr;
        u32    size;
    };
};


#endif //IFB_HPP
