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
        cmpnt_type_e_quad     = 3
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

};

#endif //IFB_COMPONENTS_HPP