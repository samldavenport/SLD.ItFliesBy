#ifndef SPRITE_INTERNAL_HPP
#define SPRITE_INTERNAL_HPP

#include "ifb-types.hpp"
#include "ifb-config.hpp"
#include "memory.hpp"

namespace ifb {

    struct sprite_list_node {
        sprite_list_node* next;
        hnd_asset         h_img_asset;
    };

    struct sprite_list {
        sprite_list_node* first; 
        sprite_list_node* last; 
    };

    struct sprite_atlas {
        struct {
            hnd_sprite_atlas h_atlas;
            u32              sprite_count;
            hnd_asset        h_asset;
            asset_name*      array_name;
            u32*             array_offset;
            dimensions_2d*   array_dims;
        } header;
        pixel* array_pixels;
    };
    
    struct sprite_mngr {
        reservation* res;
        sprite_atlas array_atlas[config::sprite_atlas_capacity];
    };


};

#endif //SPRITE_INTERNAL_HPP
