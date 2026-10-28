#ifndef IFB_ARCHETYPES_HPP
#define IFB_ARCHETYPES_HPP

#include "ifb-components.hpp"

namespace ifb {

    struct atype_sprite {
        cmpnt_position   position;
        cmpnt_color      color;
        cmpnt_quad       quad;
        cmpnt_map_coords map_coords;
        cmpnt_sprite     sprite;
    };

};

#endif //IFB_ARCHETYPES_HPP
