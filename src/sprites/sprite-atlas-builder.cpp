
#pragma once

#include "sprite.cpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"

#include <stb_rect_pack.h>

namespace ifb {

    struct sprite_atlas_builder {
        sprite_memory_block* mem_block;
        sprite_list*         list;
        image*               atlas_image;
    };

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
    
        return(builder);
    }

    IFB_INTERNAL void
    sprite_atlas_builder_destroy(
        sprite_atlas_builder* builder) {

    }

    IFB_INTERNAL void
    sprite_atlas_builder_reset(
        sprite_atlas_builder* builder) {

    }
    
    IFB_INTERNAL void
    sprite_atlas_builder_set_list(
        sprite_atlas_builder* builder,
        sprite_list*          list) {

    }

    IFB_INTERNAL void
    sprite_atlas_builder_pack_sprites(
        sprite_atlas_builder* builder) {

    }

    IFB_INTERNAL const image*
    sprite_atlas_builder_get_image(
        sprite_atlas_builder* builder) {

    }
};
