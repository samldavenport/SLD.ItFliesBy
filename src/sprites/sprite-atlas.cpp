#pragma once

#include "eng-sprite.cpp"
#include "ifb-types.hpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"

namespace ifb {

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
    
    IFB_INTERNAL sprite_atlas*
    sprite_atlas_create(
        void) {

        auto mem = sprite_mngr_get_memory();
        assert(mem);

        auto mem_block = sprite_memory_alloc(mem);
        if (!mem_block) return(NULL);

        auto atlas = (sprite_atlas*)sprite_memory_block_push(
            mem_block,
            sizeof(sprite_atlas)
        );
        assert(atlas);

        atlas->mem_block            = mem_block;
        atlas->img                  = NULL;
        atlas->header.array_handles = NULL;
        atlas->header.array_name    = NULL;
        atlas->header.array_dims    = NULL;
        atlas->header.array_offset  = NULL;
        atlas->header.sprite_count  = 0;
    
        return(atlas);
    }

    IFB_INTERNAL void
    sprite_atlas_destroy(
        sprite_atlas* atlas) {
    
        assert(atlas            != NULL);
        assert(atlas->mem_block != NULL);
   
        auto mem = sprite_mngr_get_memory();
        assert(mem);
 
        sprite_memory_block* mem_block = atlas->mem_block;

        atlas->mem_block            = NULL;
        atlas->img                  = NULL;
        atlas->header.array_handles = NULL;
        atlas->header.array_name    = NULL;
        atlas->header.array_dims    = NULL;
        atlas->header.array_offset  = NULL;
        atlas->header.sprite_count  = 0;

        sprite_memory_free(mem, mem_block);
    }

    // copies the packed image and the sprite info from the builder, so the builder can be reset or destroyed after
    // returns false if the builder can't make the image, an asset has no name, or there's no room on the block
    IFB_INTERNAL bool
    sprite_atlas_build(
        sprite_atlas*         atlas,
        sprite_atlas_builder* builder) {

        assert(atlas   != NULL);
        assert(builder != NULL);
        assert(atlas->header.sprite_count == 0);

        const auto list  = sprite_atlas_builder_get_sprite_list(builder);
        const u32  count = sprite_list_get_count(list);

        assert(count != 0);

        // this packs the sprites too, if they haven't been
        const image* builder_img = sprite_atlas_builder_get_image(builder);
        if (!builder_img) return(false);

        // the builder's image is on the builder's block, so the atlas gets its own copy
        auto img    = (image*)sprite_memory_block_push(atlas->mem_block, sizeof(image));
        auto pixels = (pixel*)sprite_memory_block_push(atlas->mem_block, builder_img->data.size);
        if (!img || !pixels) return(false);

        memcpy((void*)pixels, (const void*)builder_img->data.pixels, builder_img->data.size);
        img->data.pixels  = pixels;
        img->data.size    = builder_img->data.size;
        img->pixel_width  = builder_img->pixel_width;
        img->pixel_height = builder_img->pixel_height;

        auto array_handles =    (hnd_sprite*)sprite_memory_block_push (atlas->mem_block, count * sizeof(hnd_sprite));
        auto array_name    =    (asset_name*)sprite_memory_block_push (atlas->mem_block, count * sizeof(asset_name));
        auto array_dims    = (dimensions_2d*)sprite_memory_block_push (atlas->mem_block, count * sizeof(dimensions_2d));
        auto array_offset  =           (u32*)sprite_memory_block_push (atlas->mem_block, count * sizeof(u32));
        if (!array_handles || !array_name || !array_dims || !array_offset) return(false);

        // the arrays are in the same order as the list, which is the order the builder packed them in
        u32 sprite_index = 0;
        for (
            auto node =  sprite_list_get_first(list);
                 node != NULL;
                 node =  sprite_list_get_next(node)) {

            assert(sprite_index < count);

            const hnd_asset h_img = sprite_list_get_img_asset_hnd(node);

            const asset_name* name = asset_mngr_get_name(h_img);
            if (!name) return(false);

            const u32 offset = sprite_atlas_builder_get_sprite_offset(builder, sprite_index);
            assert(offset != INVALID_INDEX);

            // the sprite handle is the asset handle, so a sprite can be found by hashing its name
            array_handles [sprite_index] = hnd_sprite(h_img.val);
            array_name    [sprite_index] = *name;
            array_dims    [sprite_index] = sprite_list_get_img_dims(node);
            array_offset  [sprite_index] = offset;

            ++sprite_index;
        }
        assert(sprite_index == count);

        // nothing is set until everything is copied, so a failed build leaves the atlas empty
        atlas->img                  = img;
        atlas->header.array_handles = array_handles;
        atlas->header.array_name    = array_name;
        atlas->header.array_dims    = array_dims;
        atlas->header.array_offset  = array_offset;
        atlas->header.sprite_count  = count;

        return(true);
    }


    IFB_INTERNAL void                    sprite_atlas_save_to_file              (sprite_atlas* atlas, const cchar* path);
    IFB_INTERNAL void                    sprite_atlas_load_from_file             (sprite_atlas* atlas, const cchar* path); 
    IFB_INTERNAL sprite_atlas*           sprite_atlas_load_from_asset_db         (sprite_atlas* atlas, const cchar* name);
    IFB_INTERNAL u32                     sprite_atlas_get_sprite_count           (const sprite_atlas* atlas);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_name         (const sprite_atlas* atlas, const cchar*     name);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_handle       (const sprite_atlas* atlas, const hnd_sprite hnd);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_index        (const sprite_atlas* atlas, const u32        index);
};
