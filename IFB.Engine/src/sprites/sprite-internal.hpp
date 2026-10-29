#ifndef SPRITE_INTERNAL_HPP
#define SPRITE_INTERNAL_HPP

#include "ifb.hpp"
#include "ifb-config.hpp"
#include "ifb-memory.hpp"
#include "eng-sprites.hpp"
#include "eng-assets.hpp"
#include "eng-types.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct sprite_index {
        u32 width;
        u32 height;
        u32 offset;
    };

    struct sprite_memory_block;
    struct sprite_memory;

    IFB_INTERNAL sprite_memory*       sprite_mngr_get_memory    (void);

    IFB_INTERNAL sprite_memory*       sprite_memory_create               (void);
    IFB_INTERNAL void                 sprite_memory_init                 (sprite_memory* mem, reservation* res);
    IFB_INTERNAL sprite_memory_block* sprite_memory_alloc                (sprite_memory* mem);
    IFB_INTERNAL void                 sprite_memory_free                 (sprite_memory* mem, sprite_memory_block* block);
    IFB_INTERNAL u32                  sprite_memory_block_get_size_free  (sprite_memory_block* block);
    IFB_INTERNAL void*                sprite_memory_block_push           (sprite_memory_block* block, const u32 size);
    IFB_INTERNAL void                 sprite_memory_block_reset          (sprite_memory_block* block);
    IFB_INTERNAL void*                sprite_memory_block_push_all       (sprite_memory_block* block);

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
        sprite_memory_block* mem_block;
        const image*         img;
        struct {
            hnd_sprite*      array_handles;
            asset_name*      array_name;
            dimensions_2d*   array_dims;
            u32*             array_offset;
            u32              sprite_count;
        } header;
    };
};
#endif //SPRITE_INTERNAL_HPP
