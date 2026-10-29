#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>

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

// defined here instead of ifb-engine.hpp so the engine headers it includes (like eng-collections.hpp) can use it
#ifdef IFB_ENGINE_STATIC
#   define IFB_ENGINE_API
#elif defined(IFB_ENGINE_DLL_EXPORT)
#   define IFB_ENGINE_API __declspec(dllexport)
#else
#   define IFB_ENGINE_API __declspec(dllimport)
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

    //--------------------------------------------------------------------
    // HANDLES AND IDS
    //--------------------------------------------------------------------

    IFB_U32(entity_id);
    
    IFB_U32(handle);

    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_asset);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
    IFB_HANDLE(hnd_particle_emitter);
    IFB_HANDLE(hnd_sprite_atlas);
    IFB_HANDLE(hnd_sprite);

    struct text {
        cchar* cstr;
        u32    size;
    };

    struct dimensions_2d {
        f32 width;
        f32 height;
    };

    //--------------------------------------------------------------------
    // PARTICLES 
    //--------------------------------------------------------------------
    
    struct particle_emitter_info {
        const cchar*    name;
        u32             type;
        b32             is_active;   // in the list for its type
        b32             is_spawning;
        u32             count_live;
        u32             count;
        f32             lifespan_ms;
        f32             spawn_delay_ms;
        f32             spawn_radius;    // particles spawn within this distance of the position
        f32             height;
        f32             width;
        vec3            position;
        color_rgba_u32  color_primary;
        color_rgba_u32  color_secondary;
    };

    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
    };
    
    // fountains, velocity and gravity are in units per second
    struct particle_parabolic_props {
        vec3 velocity;       // launch direction and speed
        vec3 gravity;
        f32  spread_degrees; // half angle of the launch cone
    };

    // explosions, speed is in units per second
    struct particle_radial_props {
        f32 speed;
        b32 is_continuous; // spawns until stopped, instead of every particle once per start
    };
    
    //--------------------------------------------------------------------
    // MISC TYPES 
    //--------------------------------------------------------------------
};


#endif //IFB_HPP
