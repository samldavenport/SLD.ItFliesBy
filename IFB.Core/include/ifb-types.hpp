#ifndef IFB_TYPES_HPP
#define IFB_TYPES_HPP

#include <sld.hpp>

#include "ifb.hpp"

namespace ifb {
   
    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(entity_id);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct dimensions_2d {
        f32 width;
        f32 height;
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
   
};

#endif //IFB_TYPES_HPP
