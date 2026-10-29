#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRONG TYPES 
    //--------------------------------------------------------------------
    
    IFB_U32(component_type);
    
    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum cmpnt_type_e {
        cmpnt_type_e_none             = 0, 
        cmpnt_type_e_position         = bit_value(0),
        cmpnt_type_e_color            = bit_value(1),
        cmpnt_type_e_quad             = bit_value(2),
        cmpnt_type_e_velocity         = bit_value(3),
        cmpnt_type_e_acceleration     = bit_value(4),
        cmpnt_type_e_inv_mass         = bit_value(5),
        cmpnt_type_e_drag             = bit_value(6),
        cmpnt_type_e_term_velocity    = bit_value(7),
        cmpnt_type_e_spring           = bit_value(8),
        cmpnt_type_e_map_coords       = bit_value(9),
        cmpnt_type_e_particle_emitter = bit_value(10),
        cmpnt_type_e_sprite           = bit_value(11)
    };

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    using  cmpnt_position      = vec3;
    using  cmpnt_color         = color_rgba_u32;
    using  cmpnt_velocity      = vec3;
    using  cmpnt_term_velocity = vec3;

    struct cmpnt_quad {
        f32 width;
        f32 height;
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
        u32              i_sprite;
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
};

#endif //IFB_COMPONENTS_HPP
