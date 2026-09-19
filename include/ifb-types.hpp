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
#define IFB_HANDLE(hnd)   struct hnd  : handle          { using handle::handle;         };

namespace ifb {
   
    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 INVALID_U16       = 0xFFFF;
    constexpr u32 INVALID_U32       = 0xFFFFFFFF;
    constexpr u32 INVALID_INDEX     = INVALID_U32;
    constexpr u32 INVALID_HASH_32   = INVALID_U32;
    constexpr u32 ENTITY_ID_INVALID = INVALID_U32;
    constexpr u32 INVALID_HANDLE    = INVALID_U32;
    constexpr u32 INVALID_ID        = INVALID_U32;
    constexpr u32 ENTITY_TAG_SIZE   = 16;
    constexpr u16 MAP_COORD_INVALID = INVALID_U16;

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum cmpnt_type_e {
        cmpnt_type_e_none          = 0, 
        cmpnt_type_e_position      = bit_value(0),
        cmpnt_type_e_color         = bit_value(1),
        cmpnt_type_e_quad          = bit_value(2),
        cmpnt_type_e_rigid_body    = bit_value(3),
        cmpnt_type_e_velocity      = bit_value(4),
        cmpnt_type_e_acceleration  = bit_value(5),
        cmpnt_type_e_inv_mass      = bit_value(6),
        cmpnt_type_e_drag          = bit_value(7),
        cmpnt_type_e_term_velocity = bit_value(8),
        cmpnt_type_e_spring        = bit_value(9),
        cmpnt_type_e_map_coords    = bit_value(10)
    };
    
    enum map_tile_flag_e {
        map_tile_flag_e_navigable  = bit_value(0),
        map_tile_flag_e_wall_north = bit_value(1),
        map_tile_flag_e_wall_south = bit_value(2),
        map_tile_flag_e_wall_east  = bit_value(3),
        map_tile_flag_e_wall_west  = bit_value(4),
    };

    enum map_color_e {
        map_color_e_red_light    = 0,
        map_color_e_red_dark     = 1,
        map_color_e_orange_light = 2,
        map_color_e_orange_dark  = 3,
        map_color_e_yellow_light = 4,
        map_color_e_yellow_dark  = 5,
        map_color_e_green_light  = 6,
        map_color_e_green_dark   = 7,
        map_color_e_aqua_light   = 8,
        map_color_e_aqua_dark    = 9,
        map_color_e_blue_light   = 10,
        map_color_e_blue_dark    = 11,
        map_color_e_purple_light = 12,
        map_color_e_purple_dark  = 13,
        map_color_e_gray_light   = 14,
        map_color_e_gray_dark    = 15,
    };
    
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct asset;
    struct component_type;
    struct dimensions_2d;
    struct dimensions_3d;
    struct entity;
    struct entity_id;
    struct entity_tag;
    struct entity_archetype;
    struct handle;
    struct image;
    struct orientation; 
    struct strong_u32;
    struct tag;
    struct flags;

    using origin_3d        = vec3;       
    using pixel            = color_rgba_u32;

    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(handle);
    IFB_U32(entity_id);
    IFB_U32(component_type);
    IFB_U32(map_tile_flags_u32);
    IFB_U32(map_color_u32);
    
    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
   
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

    struct orientation {
        vec3 forward;
        vec3 up;
        vec3 right;
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
    
    struct map_tile {
        union {
            u32 val;
            struct {
                u16 index;
                u8  color; 
            };
        };
    };

    //--------------------------------------------------------------------
    // COMPONENT TYPES
    //--------------------------------------------------------------------
    
    using  cmpnt_position      = vec3;
    using  cmpnt_color         = color_rgba_u32;
    using  cmpnt_velocity      = vec3;
    using  cmpnt_acceleration  = vec3;
    using  cmpnt_term_velocity = vec3;
   
    struct cmpnt_quad;
    struct cmpnt_rigid_body;
    struct cmpnt_inv_mass;
    struct cmpnt_drag;
    struct cmpnt_map_coords;
    struct cmpnt_spring;
    struct cmpnt_map_coords; 

    struct cmpnt_quad {
        f32 width;
        f32 height;
    };

    struct cmpnt_rigid_body {
        vec3     origin;
        f32 width;
        f32 height;
    };

    struct cmpnt_inv_mass {
        f32 normal_val;
    };

    struct cmpnt_drag {
        f32 normal_val;
    };

    struct cmpnt_map_coords {
        f32 row_x;
        f32 col_z;
    };

    struct cmpnt_spring {
        entity_id id;
        entity_id anchor;
        f32       stiffness;
        f32       damping;
        f32       rest_length;
    };

    //--------------------------------------------------------------------
    // ARCHETYPES 
    //--------------------------------------------------------------------

    struct atype_particle {
        cmpnt_position     position;
        cmpnt_rigid_body   rigid_body;
        cmpnt_velocity     velocity;
        cmpnt_acceleration acceleration;
        cmpnt_inv_mass     inv_mass;
        cmpnt_drag         drag;
    };
    
    struct atype_quad {
        cmpnt_position   position;
        cmpnt_color      color;
        cmpnt_quad       quad;
        cmpnt_map_coords map_coods;
    };

    struct atype_physics_quad {
        cmpnt_position     position;
        cmpnt_color        color;
        cmpnt_quad         quad;
        cmpnt_rigid_body   rigid_body;
        cmpnt_velocity     velocity;
        cmpnt_acceleration acceleration;
        cmpnt_inv_mass     inv_mass;
        cmpnt_drag         drag;
        cmpnt_map_coords   map_coords;
    };
};

#endif //IFB_TYPES_HPP
