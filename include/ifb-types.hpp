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

#define IFB_U32(type)     struct type : sld::strong_u32 { using strong_u32::strong_u32; };
#define IFB_FLAGS32(type) struct type : flags           { using flags;                  };

namespace ifb {
   
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct asset;
    struct component_type;
    struct cmpnt_position;
    struct cmpnt_color;
    struct cmpnt_quad;
    struct dimensions_2d;
    struct dimensions_3d;
    struct eng_arena_handle;
    struct entity;
    struct entity_id;
    struct entity_tag;
    struct entity_archetype;
    struct handle;
    struct image;
    struct orientation; 
    struct quad;
    struct strong_u32;
    struct rigid_body;
    struct spring;
    struct tag;
    struct flags;

    using position_3d      = vec3;
    using velocity_3d      = vec3;
    using acceleration_3d  = vec3;
    using term_velocity_3d = vec3;
    using origin_3d        = vec3;       
    using pixel            = color_rgba_u32;

    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(handle);
    IFB_U32(entity_id);
    IFB_U32(component_type);

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 INVALID_INDEX     = 0xFFFFFFFF;
    constexpr u32 INVALID_HASH_32   = 0xFFFFFFFF;
    constexpr u32 ENTITY_ID_INVALID = 0xFFFFFFFF;
    constexpr u32 INVALID_HANDLE = 0xFFFFFFFF;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum tile_map_flag_e {
        tile_map_flag_e_navigable  = bit_value(0),
        tile_map_flag_e_wall_north = bit_value(1),
        tile_map_flag_e_wall_south = bit_value(2),
        tile_map_flag_e_wall_east  = bit_value(3),
        tile_map_flag_e_wall_west  = bit_value(4),
    };
    
    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct dimensions_2d {
        f32 width;
        f32 height;
    };

    struct dimensions_3d {
        f32 width;
        f32 height;
        f32 depth;
    };

    struct quad_archetype {
        position_3d    position;
        dimensions_2d  dimensions;
        color_rgba_u32 color;
    };

    struct quad {
        f32 width;
        f32 height;
    };

    struct rigid_body {
        origin_3d     origin;
        dimensions_2d dimensions;
    };

    struct inv_mass {
        f32 normal_val;
    };

    struct drag {
        f32 normal_val;
    };

    struct orientation {
        vec3 forward;
        vec3 up;
        vec3 right;
    };

    struct particle {
        rigid_body      rb;
        position        pos;
        velocity_3d     vel;
        acceleration_3d acc;
        inv_mass        im;
        drag            drg;
    };

    struct spring {
        entity_id id;
        entity_id anchor;
        f32       stiffness;
        f32       damping;
        f32       rest_length;
    };

    struct flags {

        s32 val;

        flags() = default;
        flags(s32 v) : val(v) { }

        inline bool test       (const u32 t)      { return(this->val & t); }
        inline void mask_on    (const u32 m)      { this->val |=  m;       }
        inline void mask_off   (const u32 m)      { this->val &= ~m;       }

        inline bool operator== (const u32& other) { return(this->test(other));  } 
        inline bool operator!= (const u32& other) { return(!this->test(other)); }
        inline bool operator== (const s32& other) { return(this->test(other));  } 
        inline bool operator!= (const s32& other) { return(!this->test(other)); }
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

    struct tag {
        cchar cstr[16];
        
        tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);
            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }
        inline void
        init(
            const cchar* tag_cstr)  {
            assert(cstr != NULL);
            (void)strncpy_s(cstr, 16, tag_cstr, 16);
        }

        inline u32
        hash(void) {
            return(hash_u32((void*)cstr, 16));
        }
    };
};

#endif //IFB_TYPES_HPP
