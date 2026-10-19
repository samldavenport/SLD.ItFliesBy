#pragma once

#include "sprite.cpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"
#include "asset.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // the atlas starts as the smallest power of two square that could fit, and doubles a side until it does
    static constexpr u32 SPRITE_ATLAS_BUILDER_SIZE_MAX = 4096;

    // space on the right and bottom of every sprite, so filtering doesn't bleed into its neighbors
    static constexpr u32 SPRITE_ATLAS_BUILDER_PADDING  = 1;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // everything the builder makes is pushed on its memory block
    // the rects are in the same order as the list, x and y are where each sprite is in the atlas
    struct sprite_atlas_builder {
        sprite_memory_block* mem_block;
        sprite_list*         list;
        image*               atlas_image;
        stbrp_rect*          rects;
        u32                  rect_count;
        u32                  atlas_width;
        u32                  atlas_height;
        bool                 is_packed;
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void sprite_atlas_builder_clear_results (sprite_atlas_builder* builder);
    IFB_INLINE u32  sprite_atlas_builder_pow2_ceil     (const u32 value);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL sprite_atlas_builder*
    sprite_atlas_builder_create(
        void) {

        sprite_memory* mem = sprite_mngr_get_memory();
        assert(mem);

        sprite_memory_block* mem_block = sprite_memory_alloc(mem);
        if (!mem_block) return(NULL);

        auto builder = (sprite_atlas_builder*)sprite_memory_block_push(
                mem_block,
                sizeof(sprite_atlas_builder)
        );
        assert(builder);

        builder->mem_block = mem_block;
        builder->list      = NULL;
        sprite_atlas_builder_clear_results(builder);

        return(builder);
    }

    IFB_INTERNAL void
    sprite_atlas_builder_destroy(
        sprite_atlas_builder* builder) {

        assert(builder);
        assert(builder->mem_block);

        // free the memory
        auto mem_block = builder->mem_block;
        sprite_memory* mem = sprite_mngr_get_memory();
        sprite_memory_free(mem, mem_block);

        // null the properties
        builder->mem_block = NULL;
        builder->list      = NULL;
        sprite_atlas_builder_clear_results(builder);
    }

    IFB_INTERNAL void
    sprite_atlas_builder_reset(
        sprite_atlas_builder* builder) {

        assert(builder);
        assert(builder->mem_block);

        // the builder is the first thing on the block, so it stays
        sprite_memory_block_reset(builder->mem_block);
        void* builder_mem = sprite_memory_block_push(builder->mem_block, sizeof(sprite_atlas_builder));
        assert(builder_mem == (void*)builder);

        builder->list = NULL;
        sprite_atlas_builder_clear_results(builder);
    }

    IFB_INTERNAL void
    sprite_atlas_builder_set_list(
        sprite_atlas_builder* builder,
        sprite_list*          list) {

        assert(builder);
        assert(builder->mem_block);

        // a new list needs to be packed again
        builder->list = list;
        sprite_atlas_builder_clear_results(builder);
    }

    // finds where every sprite in the list goes, the pixels aren't copied until the image is needed
    // returns false if the list is empty, or the sprites don't fit in the biggest atlas
    IFB_INTERNAL bool
    sprite_atlas_builder_pack_sprites(
        sprite_atlas_builder* builder) {

        assert(builder);
        assert(builder->mem_block);
        assert(builder->list);

        if (builder->is_packed) return(true);

        // count the sprites, and find the smallest the atlas could be
        u32 count      = 0;
        u32 max_width  = 0;
        u32 max_height = 0;
        u64 total_area = 0;
        for (
            const sprite_list_node* node = sprite_list_get_first(builder->list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            const dimensions_2d& dims   = sprite_list_get_img_dims(node);
            const u32            width  = (u32)dims.width  + SPRITE_ATLAS_BUILDER_PADDING;
            const u32            height = (u32)dims.height + SPRITE_ATLAS_BUILDER_PADDING;

            if (width  > max_width)  max_width  = width;
            if (height > max_height) max_height = height;
            total_area += (u64)width * (u64)height;
            ++count;
        }
        if (count == 0) return(false);
        if (max_width > SPRITE_ATLAS_BUILDER_SIZE_MAX || max_height > SPRITE_ATLAS_BUILDER_SIZE_MAX) return(false);

        // stb wants a node for every column of the widest atlas it may be given
        auto rects = (stbrp_rect*)sprite_memory_block_push(builder->mem_block, count * sizeof(stbrp_rect));
        auto nodes = (stbrp_node*)sprite_memory_block_push(builder->mem_block, SPRITE_ATLAS_BUILDER_SIZE_MAX * sizeof(stbrp_node));
        if (!rects || !nodes) return(false);

        u32 index = 0;
        for (
            const sprite_list_node* node = sprite_list_get_first(builder->list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            const dimensions_2d& dims = sprite_list_get_img_dims(node);

            stbrp_rect& rect = rects[index];
            rect.id         = (int)index;
            rect.w          = (stbrp_coord)((u32)dims.width  + SPRITE_ATLAS_BUILDER_PADDING);
            rect.h          = (stbrp_coord)((u32)dims.height + SPRITE_ATLAS_BUILDER_PADDING);
            rect.x          = 0;
            rect.y          = 0;
            rect.was_packed = 0;
            ++index;
        }

        // start square, then double the width and height in turns until everything fits
        u32 side_min = (u32)sqrt((f64)total_area);
        if (side_min < max_width)  side_min = max_width;
        if (side_min < max_height) side_min = max_height;

        u32  width     = sprite_atlas_builder_pow2_ceil(side_min);
        u32  height    = width;
        bool is_packed = false;

        while (width <= SPRITE_ATLAS_BUILDER_SIZE_MAX && height <= SPRITE_ATLAS_BUILDER_SIZE_MAX) {

            stbrp_context ctx;
            stbrp_init_target (&ctx, (int)width, (int)height, nodes, (int)width);
            is_packed = (stbrp_pack_rects(&ctx, rects, (int)count) == 1);
            if (is_packed) break;

            if (width <= height) width  *= 2;
            else                 height *= 2;
        }
        if (!is_packed) return(false);

        // the rects come back in the order they went in, which is the order of the list
        builder->rects        = rects;
        builder->rect_count   = count;
        builder->atlas_width  = width;
        builder->atlas_height = height;
        builder->is_packed    = true;
        return(true);
    }

    // the atlas image, with every sprite in the list copied to where it was packed
    // the sprites are packed first if they haven't been, and the image is only made once
    // returns NULL if the sprites can't be packed, an image can't be loaded, or there's no room on the block
    IFB_INTERNAL const image*
    sprite_atlas_builder_get_image(
        sprite_atlas_builder* builder) {

        assert(builder);
        assert(builder->mem_block);
        assert(builder->list);

        if (builder->atlas_image) return(builder->atlas_image);
        if (!sprite_atlas_builder_pack_sprites(builder)) return(NULL);

        const u32 width       = builder->atlas_width;
        const u32 height      = builder->atlas_height;
        const u32 pixel_count = width * height;
        const u32 size_pixels = pixel_count * sizeof(pixel);

        auto atlas_image  = (image*)sprite_memory_block_push(builder->mem_block, sizeof(image));
        auto atlas_pixels = (pixel*)sprite_memory_block_push(builder->mem_block, size_pixels);
        if (!atlas_image || !atlas_pixels) return(NULL);

        // anything that isn't a sprite is transparent
        memset((void*)atlas_pixels, 0, size_pixels);

        u32 index = 0;
        for (
            const sprite_list_node* node = sprite_list_get_first(builder->list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            assert(index < builder->rect_count);
            const stbrp_rect& rect = builder->rects[index];
            assert(rect.was_packed);

            // the pointer is only good until the next load, so each image is copied before the next one is loaded
            const asset_image* img = asset_mngr_get_image(sprite_list_get_img_asset_hnd(node));
            if (!img) return(NULL);

            // the list kept the size when the sprite was added, and that's what was packed
            const dimensions_2d& dims = sprite_list_get_img_dims(node);
            if (img->width != (u32)dims.width || img->height != (u32)dims.height) return(NULL);

            // both images go from the top row down
            const u32 size_row = img->width * sizeof(pixel);
            for (
                u32 row = 0;
                    row < img->height;
                  ++row) {

                const pixel* src = &img->pixels  [row * img->width];
                pixel*       dst = &atlas_pixels [((rect.y + row) * width) + rect.x];
                memcpy((void*)dst, (const void*)src, size_row);
            }

            ++index;
        }

        atlas_image->data.pixels  = atlas_pixels;
        atlas_image->data.size    = size_pixels;
        atlas_image->pixel_width  = width;
        atlas_image->pixel_height = height;

        builder->atlas_image = atlas_image;
        return(builder->atlas_image);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the results are left on the block, they go away when it's reset
    IFB_INLINE void
    sprite_atlas_builder_clear_results(
        sprite_atlas_builder* builder) {

        builder->atlas_image  = NULL;
        builder->rects        = NULL;
        builder->rect_count   = 0;
        builder->atlas_width  = 0;
        builder->atlas_height = 0;
        builder->is_packed    = false;
    }

    IFB_INLINE u32
    sprite_atlas_builder_pow2_ceil(
        const u32 value) {

        u32 pow2 = 1;
        while (pow2 < value) {
            pow2 *= 2;
        }
        return(pow2);
    }
};
