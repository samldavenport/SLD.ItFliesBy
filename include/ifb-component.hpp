#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    enum cmpnt_type_e {
        cmpnt_type_e_none          = 0,
        cmpnt_type_e_position      = 1,
        cmpnt_type_e_color         = 2,
        cmpnt_type_e_quad          = 3,
        cmpnt_type_e_rigid_body    = 4,
        cmpnt_type_e_velocity      = 5,
        cmpnt_type_e_acceleration  = 6,
        cmpnt_type_e_inv_mass      = 7,
        cmpnt_type_e_drag          = 8,
        cmpnt_type_e_term_velocity = 9
    };

    struct cmpnt_base {
        entity_id id;
        u32       sparse_index;
    };

    struct cmpnt_position : cmpnt_base {
        f32       x;
        f32       y;
        f32       z;
    };

    struct cmpnt_color : cmpnt_base {
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

    struct cmpnt_quad : cmpnt_base {
        u32 width;
        u32 height;
    };

    struct cmpnt_rigid_body : cmpnt_base {
        f32 origin_x;
        f32 origin_y;
        f32 origin_z;
        f32 width;
        f32 height;
    };

    struct cmpnt_velocity : cmpnt_base {
        f32 x;
        f32 y;
        f32 z;
    };

    struct cmpnt_acceleration : cmpnt_base {
        f32 x;
        f32 y;
        f32 z;
    };

    struct cmpnt_inv_mass : cmpnt_base {
        f32 val;
    };

    struct cmpnt_drag : cmpnt_base {
        f32 val;
    };

    struct cmpnt_term_velocity : cmpnt_base {
        f32 x;
        f32 y;
        f32 z;
    };
};

#endif //IFB_COMPONENTS_HPP
