#ifndef IFB_TYPES_HPP
#define IFB_TYPES_HPP

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>
#include <sld-strings.hpp>

#include "ifb-config.hpp"

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
    constexpr f32 POS_MAX           =  1.0f;
    constexpr f32 POS_MIN           = -1.0f;

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum cmpnt_type_e {
        cmpnt_type_e_none             = 0, 
        cmpnt_type_e_position         = bit_value(0),
        cmpnt_type_e_color            = bit_value(1),
        cmpnt_type_e_quad             = bit_value(2),
        cmpnt_type_e_rigid_body       = bit_value(3),
        cmpnt_type_e_velocity         = bit_value(4),
        cmpnt_type_e_acceleration     = bit_value(5),
        cmpnt_type_e_inv_mass         = bit_value(6),
        cmpnt_type_e_drag             = bit_value(7),
        cmpnt_type_e_term_velocity    = bit_value(8),
        cmpnt_type_e_spring           = bit_value(9),
        cmpnt_type_e_map_coords       = bit_value(10),
        cmpnt_type_e_particle_emitter = bit_value(11),
        cmpnt_type_e_sprite           = bit_value(12)
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
   
    enum asset_type_e {
        asset_type_e_text          = 0,
        asset_type_e_image         = 1,
        asset_type_e_sprite_atlas  = 2,
        asset_type_e_count         = 3 
    };

    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
    };

    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(handle);
    IFB_U32(entity_id);
    IFB_U32(component_type);
    IFB_U32(map_tile_flags_u32);
    IFB_U32(map_color_u32);
    IFB_U32(asset_type_u32);
    
    IFB_HANDLE(hnd_asset);
    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
    IFB_HANDLE(hnd_particle_emitter);
    IFB_HANDLE(hnd_sprite_atlas);
    IFB_HANDLE(hnd_sprite);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    using origin_3d        = vec3;       
    using pixel            = color_rgba_u32;

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

    struct particle {
        color_rgba_u32 color;
        vec3           position;
    };


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
    // ASSET TYPES
    //--------------------------------------------------------------------
    
    struct asset_name {
        cchar cstr [IFB_CONFIG_ASSET_NAME_SIZE];
        
        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            (void)cstr_nvar_copy(
                cstr,
                IFB_CONFIG_ASSET_NAME_SIZE,
                this->cstr,
                IFB_CONFIG_ASSET_NAME_SIZE
            );
        }

        inline hnd_asset
        to_handle(void) {

            const hnd_asset hnd = hash_u32((void*)cstr, IFB_CONFIG_ASSET_NAME_SIZE);
            assert(hnd != INVALID_HANDLE);

            return(hnd);
        }
    };

    struct text {
        cchar* cstr;
        u32    size;
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
    };
    
    // the strings belong to the dictionary
    struct asset_dictionary_file_info {
        const cchar* name;
        const cchar* path;
        u32          id;
        u32          type;
    };

    // the name may use every byte, so this one has room for a terminator
    struct asset_database_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_asset hnd;
        u32       type;
        u32       offset;
        u32       size;
    };

    // a sprite in the atlas builder list, the size is in pixels
    struct sprite_atlas_builder_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_asset hnd;
        u32       width;
        u32       height;
    };

    // the atlas open in the inspector, the size is in pixels
    struct sprite_atlas_info {
        u32 width;
        u32 height;
        u32 sprite_count;
    };

    // a sprite in an atlas, x and y are its top left corner in the atlas, everything is in pixels
    struct sprite_atlas_sprite_info {
        cchar      name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_sprite hnd;
        u32        x;
        u32        y;
        u32        width;
        u32        height;
    };

    // the contents of a dictionary file or database asset, only good until the next preview from the same place
    // a text file has the text, an image file has an opengl texture id
    struct asset_preview {
        u32          type;
        const cchar* text;
        b32          is_text_truncated; // the file is bigger than the file buffer
        u32          texture;
        u32          width;
        u32          height;
    };

    struct asset_text {
        hnd_asset hnd;
        u32       size;
        cchar*    cstr;
    };

    struct asset_image {
        hnd_asset       hnd;
        u32             width;
        u32             height;
        color_rgba_u32* pixels;
    };

    struct sprite {
        const cchar* name;
        const pixel* pixel_array; 
        hnd_sprite   hnd;
        u32          index;
        u32          pixel_width;
        u32          pixel_height;
    };

    struct sprite_index {
        u32 width;
        u32 height;
        u32 offset;
    };
    
    struct asset_sprite_atlas {
        hnd_sprite*   array_handles;
        sprite_index* array_indexes;
        pixel*        array_pixels;
        hnd_asset     hnd;
        u32           width;
        u32           height;
        u32           sprite_count;
    };


    //--------------------------------------------------------------------
    // COMPONENT TYPES
    //--------------------------------------------------------------------
    
    using  cmpnt_position      = vec3;
    using  cmpnt_color         = color_rgba_u32;
    using  cmpnt_velocity      = vec3;
    using  cmpnt_acceleration  = vec3;
    using  cmpnt_term_velocity = vec3;
   
    struct cmpnt_quad {
        f32 width;
        f32 height;
    };

    struct cmpnt_rigid_body {
        vec3 origin;
        f32  width;
        f32  height;
    };

    struct cmpnt_inv_mass {
        f32 normal_val;
    };

    struct cmpnt_drag {
        f32 normal_val;
    };

    // the position component is the source of truth for where an entity is
    // h_map is the map the entity belongs to, the rest is the position
    // measured in tiles from the map origin: columns along x, levels along y, rows along z
    struct cmpnt_map_coords {
        hnd_map h_map;
        f32     col_x;
        f32     lvl_y;
        f32     row_z;
    };

    struct cmpnt_spring {
        entity_id id;
        entity_id anchor;
        f32       stiffness;
        f32       damping;
        f32       rest_length;
    };
    
    struct cmpnt_particle_emitter {
        hnd_particle_emitter hnd;
    };

    struct cmpnt_sprite {
        hnd_sprite_atlas h_atlas;
        u32              index; 
    };

    //--------------------------------------------------------------------
    // ARCHETYPES 
    //--------------------------------------------------------------------

    struct atype_particle {
        cmpnt_position      position;
        cmpnt_rigid_body    rigid_body;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
    };
    
    struct atype_quad {
        cmpnt_position   position;
        cmpnt_color      color;
        cmpnt_quad       quad;
        cmpnt_map_coords map_coords;
    };

    struct atype_physics_quad {
        cmpnt_position      position;
        cmpnt_color         color;
        cmpnt_quad          quad;
        cmpnt_rigid_body    rigid_body;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
        cmpnt_map_coords    map_coords;
    };
    
    //--------------------------------------------------------------------
    // SRITE TYPES
    //--------------------------------------------------------------------
};

#endif //IFB_TYPES_HPP
