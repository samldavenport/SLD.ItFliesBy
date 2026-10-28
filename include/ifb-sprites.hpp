#ifndef IFB_SPRITES_HPP
#define IFB_SPRITES_HPP

#include "ifb-types.hpp"
#include "ifb-assets.hpp"

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

    struct sprite_index {
        u32 width;
        u32 height;
        u32 offset;
    };
    
    struct sprite_atlas_info {
        u32 width;
        u32 height;
        u32 sprite_count;
    };

    struct sprite_atlas_sprite_info {
        cchar      name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_sprite hnd;
        u32        x;
        u32        y;
        u32        width;
        u32        height;
    };
};

#endif //IFB_SPRITES_HPP
