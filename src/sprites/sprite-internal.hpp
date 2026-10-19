#ifndef SPRITE_INTERNAL_HPP
#define SPRITE_INTERNAL_HPP

#include "ifb-types.hpp"

namespace ifb {

    struct sprite_atlas {
        hnd_sprite_atlas h_atlas;
        u32              sprite_count;
        hnd_sprite*      array_hnd_sprite;
        dimensions_2d*   array_dims;
        pixel*           array_pixels;
    };

};

#endif //SPRITE_INTERNAL_HPP
