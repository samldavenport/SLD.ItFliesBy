#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"
#include "ifb-entity.hpp"

namespace ifb {

    enum cmpnt_type_e {
        cmpnt_type_e_none         = 0,
        cmpnt_type_e_position     = 1,
        cmpnt_type_e_color        = 2,
        cmpnt_type_e_quad         = 3,
        cmpnt_type_e_rigid_body   = 4,
        cmpnt_type_e_velocity     = 5,
        cmpnt_type_e_acceleration = 6,
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

    struct cmpnt_rigid_body {
        entity_id id;
        f32       origin_x;
        f32       origin_y;
        f32       origin_z;
        f32       width;
        f32       height;
    };

    struct cmpnt_velocity {
        entity_id id;
        f32 x;
        f32 y;
        f32 z;
    };

    struct cmpnt_acceleration {
        entity_id id;
        f32 x;
        f32 y;
        f32 z;
    };

};

#endif //IFB_COMPONENTS_HPP