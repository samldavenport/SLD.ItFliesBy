#ifndef ENG_TYPES_HPP
#define ENG_TYPES_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(map_color_u32);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    using pixel            = color_rgba_u32;

    struct orientation {
        vec3 forward;
        vec3 up;
        vec3 right;
    };

    struct image {
        struct {
            union {
                pixel* pixels;
                byte*  bytes;
                void*  vptr;
                addr   address;
            };
            u32 size;
        } data;
        u32 pixel_width;
        u32 pixel_height;
    };

    struct particle {
        color_rgba_u32 color;
        vec3           position;
    };
};

#endif //ENG_TYPES_HPP
