#ifndef ENG_ARCHETYPES_HPP
#define ENG_ARCHETYPES_HPP

#include "ifb-archetypes.hpp"
#include "eng-components.hpp"
#include "ifb-components.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct atype_particle {
        cmpnt_position      position;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
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

#endif //ENG_ARCHETYPES_HPP
