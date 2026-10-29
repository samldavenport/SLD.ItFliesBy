#ifndef ENG_SPRITES_HPP
#define ENG_SPRITES_HPP

#include "ifb-sprites.hpp"
#include "eng-assets.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct sprite_info {
        asset_name   name;
        hnd_sprite   hnd;
        u32          index;
        u32          pixel_x;
        u32          pixel_y;
        u32          pixel_width;
        u32          pixel_height;
        u32          atlas_width;
        u32          atlas_height;
    };
};

#endif //ENG_SPRITES_HPP
