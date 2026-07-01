#ifndef IFB_TYPES_HPP
#define IFB_TYPES_HPP

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

using namespace sld;

#define IFB_INTERNAL static
#define IFB_INLINE   inline

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif

namespace ifb {
    
    struct asset;
    struct component_type;
    struct component_position;
    struct component_color;
    struct dimensions_2d;
    struct dimensions_3d;
    struct eng_arena_handle;
    struct entity;
    struct entity_id;
    struct entity_tag;
    struct entity_archetype;
    struct handle;
    struct handle;
    struct image;
    struct quad;

    using position_3d = vec3;
    using pixel       = color_rgba_u32;

    constexpr u32 INVALID_INDEX   = 0xFFFFFFFF;
    constexpr u32 INVALID_HASH_32 = 0xFFFFFFFF;


    struct handle {

        u32 val;

        handle(void) = default;
        handle(u32 v) : val(v) { }

        inline bool operator== (const u32& other) { return(other == val); } 
        inline bool operator!= (const u32& other) { return(other != val); }
    };

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

    struct image {
        struct {
            union {
                pixel* pixels;
                byte*  bytes;
                void*  vptr;
                addr   address;
            };
            u32 size;
        } data;
        u32 pixel_width;
        u32 pixel_height;
        u32 pixel_count;
    };
};

#endif //IFB_TYPES_HPP