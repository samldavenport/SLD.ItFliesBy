#ifndef TYPES_INTERNAL_HPP
#define TYPES_INTERNAL_HPP

#include "ifb.hpp"
#include "eng-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRONG TYPES
    //--------------------------------------------------------------------

    IFB_U32(asset_type_u32);
    IFB_U8(map_tile_flags_u8);
    IFB_U8(map_wall_flags_u8);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    using origin_3d        = vec3;       

    struct dimensions_3d {
        f32 width;
        f32 height;
        f32 depth;
    };

    struct map_tile {
        union {
            u32 val;
            struct {
                u16 index;
                u8  color;
                map_tile_flags_u8 flags; // map_tile_flag_e
            };
        };
        f32 u_left;
        f32 v_top;
        f32 u_right;
        f32 v_bottom;
    };
};

#endif //TYPES_INTERNAL_HPP
