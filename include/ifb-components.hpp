#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"
#include "ifb-entity.hpp"

namespace ifb {

    enum component_type_e {
        component_type_e_none     = 0,
        component_type_e_position = 1,
        component_type_e_color    = 2
    };

    struct component_position {
        entity_id id;
        f32       x;
        f32       y;
        f32       z;
    };

    struct component_color {
        entity_id id;
        union {
            u32 hex_rgba;
            struct {
                byte a;
                byte b;
                byte g;
                byte r;
            };
        };
    };

};

#endif //IFB_COMPONENTS_HPP