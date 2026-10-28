#ifndef IFB_TYPES_HPP
#define IFB_TYPES_HPP

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>
#include <sld-strings.hpp>

#include "ifb.hpp"
#include "ifb-config.hpp"



namespace ifb {
   
    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum map_tile_flag_e {
        map_tile_flag_e_textured    = bit_value(1), 
        map_tile_flag_e_navigable   = bit_value(0),
        map_tile_flag_e_wall_row    = bit_value(2), // render only, the tile is a row wall
        map_tile_flag_e_wall_column = bit_value(3), // render only, the tile is a column wall
    };

    enum map_wall_flag_e {
        map_wall_flag_e_placed   = bit_value(0),
        map_wall_flag_e_textured = bit_value(1),
    };

    // row walls lie on the line z = row and run along x, on the min-z edge of tile (row, col)
    // column walls lie on the line x = col and run along z, on the min-x edge of tile (row, col)
    enum map_wall_orientation_e {
        map_wall_orientation_e_row    = 0,
        map_wall_orientation_e_column = 1,
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
        map_color_e_none         = 16,
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
    IFB_U32(map_color_u32);
    IFB_U32(asset_type_u32);
    
    IFB_U8(map_tile_flags_u8);
    IFB_U8(map_wall_flags_u8);

    IFB_HANDLE(hnd_arena);
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

    struct map_tile {
        union {
            u32 val;
            struct {
                u16 index;
                u8  color;
                map_tile_flags_u8 flags; // map_tile_flag_e
            };
        };
        f32 u_left;
        f32 v_top;
        f32 u_right;
        f32 v_bottom;
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
    


};

#endif //IFB_TYPES_HPP
