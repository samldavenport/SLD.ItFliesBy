#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>
#include "ifb-entity.hpp"

namespace ifb {

    struct component {
        entity_id id;
    };  

    struct position_component : component {
        u32 x;
        u32 y;
        u32 z;
    };

    struct color_component : component {
        union {
            struct {
                byte r;
                byte g;
                byte b;
                byte a;
            };
            u32 hex;
        };
    };

    struct velocity_component : component {
        f32 x;
        f32 y;
        f32 z;
    };

    struct dimension_component : component {
        u32 width;
        u32 height;
    };

    struct rotation_component : component {
        f32 x;
        f32 y;
        f32 z;
    };

    struct scale_component : component {
        f32 x;
        f32 y;
        f32 z;
    };

    struct quad_component : component {
        u32 color_hex;
        u32 pos_x;
        u32 pos_y;
        u32 pos_z;
        u32 width;
        u32 height;
        f32 scale_x;
        f32 scale_y;
        f32 rotation_rad;
    };
};

#endif //IFB_COMPONENTS_HPP