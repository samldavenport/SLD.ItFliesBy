#ifndef IFB_ARCHETYPES_HPP
#define IFB_ARCHETYPES_HPP

#include "ifb.hpp"
#include "ifb-components.hpp"
#include "ifb-map.hpp"

namespace ifb {

    struct atype_particle {
        cmpnt_position      position;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
    };
    
    struct atype_sprite {
        cmpnt_position   position;
        cmpnt_color      color;
        cmpnt_quad       quad;
        cmpnt_map_coords map_coords;
        cmpnt_sprite     sprite;
    };

    struct atype_physics_sprite {
        cmpnt_position      position;
        cmpnt_color         color;
        cmpnt_quad          quad;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
        cmpnt_map_coords    map_coords;
        cmpnt_sprite        sprite;
    };
};


#endif //IFB_ARCHETYPES_HPP
