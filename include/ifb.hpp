#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "ifb-engine.hpp"
#include "ifb-input.hpp"
#include "ifb-font-opensans.hpp"

#define IFB_INTERNAL static

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif

using namespace sld;

extern IMGUI_IMPL_API LRESULT 
ImGui_ImplWin32_WndProcHandler(
    HWND   hWnd, 
    UINT   msg, 
    WPARAM wParam, 
    LPARAM lParam);


namespace ifb {

    struct sparse_array;
    struct quad;
    struct dimensions_2d;
    struct dimensions_3d;

    using position_3d = vec3;

    constexpr u32 INVALID_INDEX   = 0xFFFFFFFF;
    constexpr u32 INVALID_HASH_32 = 0xFFFFFFFF;

    IFB_API u32           sparse_array_memory_requirement (const u32 capacity, const u32 val_size, const f32 max_load_p100);
    IFB_API sparse_array* sparse_array_memory_init        (const u32 capacity, const u32 val_size, const f32 max_load_p100, const u32 key_size, const u32 mem_size, void* mem_ptr);
    IFB_API bool          sparse_array_is_valid           (const sparse_array* sa);
    IFB_API void          sparse_array_assert_valid       (const sparse_array* sa);
    IFB_API void*         sparse_array_lookup             (const sparse_array* sa, const cchar8* key);
    IFB_API bool          sparse_array_insert             (sparse_array* sa, const cchar8* key, const void* val);
    IFB_API bool          sparse_array_remove             (sparse_array* sa, const cchar8* key);

    struct dimensions_2d {
        f32 width;
        f32 height;
    };

    struct dimensions_3d {
        f32 width;
        f32 height;
        f32 depth;
    };

    struct quad {
        position_3d    position;
        dimensions_2d  dimensions;
        color_rgba_u32 color;
    };

    struct flags {

        s32 val;

        flags() = default;
        flags(s32 v) : val(v) { }

        inline bool test     (const u32 t) { return(this->val & t); }
        inline void mask_on  (const u32 m) { this->val |=  m;       }
        inline void mask_off (const u32 m) { this->val &= ~m;       }

        inline bool   operator== (const u32& other) { return(this->test(other));  } 
        inline bool   operator!= (const u32& other) { return(!this->test(other)); }
        inline bool   operator== (const s32& other) { return(this->test(other));  } 
        inline bool   operator!= (const s32& other) { return(!this->test(other)); }
    };
};

#endif //IFB_HPP