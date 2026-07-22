#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"
#include "ifb-entity.hpp"

namespace ifb {

    enum cmpnt_type_e {
        cmpnt_type_e_none     = 0,
        cmpnt_type_e_position = 1,
        cmpnt_type_e_color    = 2,
        cmpnt_type_e_quad     = 3,
        cmpnt_type_e_physics  = 4,

    };

    enum cmpnt_physics_type_e {
        cmpnt_physics_type_e_static  = 0,
        cmpnt_physics_type_e_dynamic = 1
    };

    struct cmpnt_position {
        entity_id id;
        u32       sparse_index;
        f32       x;
        f32       y;
        f32       z;
    };

    struct cmpnt_color {
        entity_id id;
        u32       sparse_index;
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

    struct cmpnt_quad {
        entity_id id;
        u32       sparse_index;
        u32       width;
        u32       height;
    };

    struct cmpnt_physics {
        entity_id     id;
        u32           type;
        position_3d   origin;
        dimensions_2d dimensions;
    };

    struct cmpnt_motion {
        entity_id       id;
        velocity_3d     velocity;
        acceleration_3d acceleration;
    };
};

#endif //IFB_COMPONENTS_HPP