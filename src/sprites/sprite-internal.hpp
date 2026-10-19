#ifndef SPRITE_INTERNAL_HPP
#define SPRITE_INTERNAL_HPP

#include "ifb-types.hpp"
#include "ifb-config.hpp"
#include "memory.hpp"

namespace ifb {


    struct sprite_memory_block;
    struct sprite_memory;

    IFB_INTERNAL sprite_memory*       sprite_mngr_get_memory   (void);

    IFB_INTERNAL sprite_memory*       sprite_memory_create     (void);
    IFB_INTERNAL void                 sprite_memory_init       (sprite_memory* mem, reservation* res);
    IFB_INTERNAL sprite_memory_block* sprite_memory_alloc      (sprite_memory* mem);
    IFB_INTERNAL void                 sprite_memory_free       (sprite_memory* mem, sprite_memory_block* block);
    IFB_INTERNAL void*                sprite_memory_block_push (sprite_memory_block* block, const u32 size);
    IFB_INTERNAL void                 sprite_memory_reset      (sprite_memory_block* block);

    struct sprite_list_node {
        sprite_list_node* next;
        hnd_asset         h_img_asset;
        dimensions_2d     img_dims;
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
        sprite_memory* mem;
    };

    
};

#endif //SPRITE_INTERNAL_HPP
