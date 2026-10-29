#pragma once

#include "eng-internal.hpp"
#include "ifb.hpp"
#include "sld.hpp"
#include "sprite-internal.hpp"
#include "sprites.hpp"
#include "asset.hpp"
#include "memory-internal.hpp"
#include "asset-internal.hpp"

namespace ifb {

    struct sprite_atlas_table {
        hnd_sprite_atlas          array_hnd   [config::sprite_atlas_table_capacity]; 
        const asset_sprite_atlas* array_atlas [config::sprite_atlas_table_capacity];              
        sprite_memory_block*      array_mem   [config::sprite_atlas_table_capacity];              
    };

    struct sprite_mngr {
        sprite_memory*      mem;
        sprite_atlas_table* atlas_tbl;   
    } static * _sprite_mngr;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline const asset_sprite_atlas* sprite_mngr_find_atlas        (const hnd_sprite_atlas h_atlas);
    inline u32                       sprite_mngr_find_sprite_index (const asset_sprite_atlas* atlas, const hnd_sprite h_sprite);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL sprite_mngr*
    sprite_mngr_create(
        void) {

        _sprite_mngr = (sprite_mngr*)global_alloc(sizeof(sprite_mngr)); 
        assert(_sprite_mngr);

        _sprite_mngr->mem = sprite_memory_create();
        assert(_sprite_mngr->mem);

        _sprite_mngr->atlas_tbl = (sprite_atlas_table*)global_alloc(sizeof(sprite_atlas_table));
        assert(_sprite_mngr->atlas_tbl);        

        return(_sprite_mngr);
    }

    IFB_INTERNAL void
    sprite_mngr_startup(
        reservation* res) {

        assert(_sprite_mngr);
        assert(_sprite_mngr->mem);
        assert(res);

        sprite_memory_init(_sprite_mngr->mem, res);
   
        auto tbl = _sprite_mngr->atlas_tbl;
        for (
            u32 i_tbl = 0;
                i_tbl < config::sprite_atlas_table_capacity;
              ++i_tbl) {

            tbl->array_hnd[i_tbl] = INVALID_HANDLE;
        }
    }
    
    IFB_INTERNAL hnd_sprite_atlas
    sprite_mngr_load_atlas(
        const cchar* atlas_name_cstr) {

        assert(atlas_name_cstr);
        assert(_sprite_mngr);
        assert(_sprite_mngr->atlas_tbl);
  
        // the atlas handle is the asset handle
        const hnd_sprite_atlas h_atlas = hnd_sprite_atlas(asset_name_cstr_to_handle(atlas_name_cstr).val);
        if (h_atlas == INVALID_HANDLE) {
            return(INVALID_HANDLE);
        }

        // cache the table
        auto tbl = _sprite_mngr->atlas_tbl;

        // if the atlas is already loaded, return it
        // otherwise, find an available index
        u32 i_tbl = INVALID_INDEX;
        for (
              u32 i_tbl_curr = 0;
                  i_tbl_curr < config::sprite_atlas_table_capacity;
                ++i_tbl_curr) {

            const hnd_sprite_atlas h_curr = tbl->array_hnd[i_tbl_curr];
            if (h_curr == h_atlas) {
                return(h_atlas);
            }
            if (h_curr == INVALID_HANDLE && i_tbl == INVALID_INDEX) {
                i_tbl = i_tbl_curr;
            }
        }

        // if we didn't find one, we're done
        if (i_tbl == INVALID_INDEX) {
            return(INVALID_HANDLE);
        }

        // allocate a block of memory for the atlas
        sprite_memory_block* block = sprite_memory_alloc(_sprite_mngr->mem);  
        if (block == NULL) {
            return(INVALID_HANDLE);
        }

        // get all the memory in the block
        memory mem;
        mem.size = sprite_memory_block_get_size_free (block);
        mem.ptr  = sprite_memory_block_push_all      (block);
        assert(
            mem.size    != 0 &&
            mem.address != 0
        );

        // load the atlas to the memory block
        // the sprite manager will own the atlas,
        // not the asset manager
        asset_database* db = asset_mngr_get_database();
        assert(db);
        const asset_sprite_atlas* atlas = asset_database_load_sprite_atlas_by_name_to_memory(
            db,
            atlas_name_cstr,
            mem
        );

        // if the load failed, give the block back
        if (atlas == NULL) {
            sprite_memory_free(_sprite_mngr->mem, block);
            return(INVALID_HANDLE);
        }

        // add the details from this atlas to the table
        tbl->array_atlas [i_tbl] = atlas;
        tbl->array_hnd   [i_tbl] = atlas->hnd;
        tbl->array_mem   [i_tbl] = block;
   
        // return the atlas handle
        return(atlas->hnd);
    }

    IFB_INTERNAL hnd_sprite_atlas
    sprite_mngr_lookup_atlas(
        const cchar* atlas_name_cstr) {

        assert(atlas_name_cstr != NULL);

        const hnd_sprite_atlas h_atlas = { asset_name_cstr_to_handle(atlas_name_cstr).val };
        assert(h_atlas != INVALID_HANDLE); 

        const auto* atlas = sprite_mngr_find_atlas(h_atlas);
        return(atlas != NULL ? h_atlas : INVALID_HANDLE);
    }
    
    IFB_INTERNAL bool
    sprite_mngr_unload_atlas(
        const hnd_sprite_atlas h_atlas) {

        assert(_sprite_mngr);
        assert(_sprite_mngr->atlas_tbl);

        if (h_atlas == INVALID_HANDLE) {
            return(false);
        }

        // cache the table
        auto tbl = _sprite_mngr->atlas_tbl;

        // find the atlas
        for (
              u32 i_tbl = 0;
                  i_tbl < config::sprite_atlas_table_capacity;
                ++i_tbl) {

            if (tbl->array_hnd[i_tbl] != h_atlas) {
                continue;
            }

            // give the block back and clear the slot
            sprite_memory_free(_sprite_mngr->mem, tbl->array_mem[i_tbl]);
            tbl->array_hnd   [i_tbl] = INVALID_HANDLE;
            tbl->array_atlas [i_tbl] = NULL;
            tbl->array_mem   [i_tbl] = NULL;

            return(true);
        }

        // the atlas isn't loaded
        return(false);
    }

    IFB_INTERNAL bool
    sprite_mngr_get_atlas_info(
        const hnd_sprite_atlas h_atlas,
        sprite_atlas_info&     info) {

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL) {
            return(false);
        }

        info.sprite_count = atlas->sprite_count;
        info.width        = atlas->width;
        info.height       = atlas->height;

        return(true);
    }   

    IFB_INTERNAL hnd_sprite
    sprite_mngr_lookup_sprite_handle(
        const hnd_sprite_atlas h_atlas,
        const cchar*           sprite_name_cstr) {

        // we know what it is ahead of time,
        // but we are also verifying it exists
        const u32 i_sprite = sprite_mngr_lookup_sprite_index(h_atlas, sprite_name_cstr);
        if (i_sprite == INVALID_INDEX) {
            return(INVALID_HANDLE);
        }

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        return(atlas->array_handles[i_sprite]);
    }

    IFB_INTERNAL u32
    sprite_mngr_lookup_sprite_index(
        const hnd_sprite_atlas h_atlas,
        const cchar*           sprite_name_cstr) {

        assert(sprite_name_cstr != NULL);

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL) {
            return(INVALID_INDEX);
        }

        const hnd_sprite h_sprite = {
            asset_name_cstr_to_handle(sprite_name_cstr).val
        };

        return(sprite_mngr_find_sprite_index(atlas, h_sprite));
    }
   
    IFB_INTERNAL bool
    sprite_mngr_get_sprite_info_by_handle(
        const hnd_sprite_atlas h_atlas,
        const hnd_sprite       h_sprite,
        sprite_info&           info) {

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL) {
            return(false);
        }

        const u32 i_sprite = sprite_mngr_find_sprite_index(atlas, h_sprite);
        if (i_sprite == INVALID_INDEX) {
            return(false);
        }

        return(sprite_mngr_get_sprite_info_by_index(h_atlas, i_sprite, info));
    }

    IFB_INTERNAL bool
    sprite_mngr_get_sprite_info_by_index(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        sprite_info&           info) {

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL || i_sprite >= atlas->sprite_count) {
            return(false);
        }

        const auto& index = atlas->array_indexes[i_sprite];

        info.hnd          = atlas->array_handles[i_sprite];
        info.pixel_width  = index.width;
        info.pixel_height = index.height;
        info.index        = i_sprite;

        // the offset is into the atlas pixels, row major
        info.atlas_width  = atlas->width;
        info.atlas_height = atlas->height;
        info.pixel_x      = index.offset % atlas->width;
        info.pixel_y      = index.offset / atlas->width;

        // the sprite handle is the handle of its image asset, so the name comes from the database
        // the name is all zeros if the image isn't in the database
        const asset_name* name = asset_mngr_get_name(hnd_asset(info.hnd.val));
        if (name != NULL) {
            info.name = *name;
        }
        else {
            info.name = {};
        }

        return(true);
    }

    IFB_INTERNAL image*
    sprite_mngr_get_sprite_image_by_handle(
        const hnd_sprite_atlas h_atlas,
        const hnd_sprite       h_sprite,
        const hnd_arena        h_arena) {

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL) {
            return(NULL);
        }

        const u32 i_sprite = sprite_mngr_find_sprite_index(atlas, h_sprite);
        if (i_sprite == INVALID_INDEX) {
            return(NULL);
        }

        return(sprite_mngr_get_sprite_image_by_index(h_atlas, i_sprite, h_arena));
    }

    // copies the sprite out of the atlas into a new image on the arena
    // the pixels go right after the struct, the same as an image loaded to an arena
    // NULL if the sprite doesn't exist or there's no room in the arena
    IFB_INTERNAL image*
    sprite_mngr_get_sprite_image_by_index(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        const hnd_arena        h_arena) {

        assert(h_arena != INVALID_HANDLE);

        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL || i_sprite >= atlas->sprite_count) {
            return(NULL);
        }

        const auto& index = atlas->array_indexes[i_sprite];

        // allocate the image and its pixels together
        const u32 size_row    = index.width * sizeof(pixel);
        const u32 size_pixels = size_row    * index.height;
        auto      img         = (image*)arena_push(h_arena, sizeof(image) + size_pixels);
        if (img == NULL) {
            return(NULL);
        }

        img->data.address = ((addr)img) + sizeof(image);
        img->data.size    = size_pixels;
        img->pixel_width  = index.width;
        img->pixel_height = index.height;

        // the sprite's rows are an atlas width apart
        for (
            u32 row = 0;
                row < index.height;
              ++row) {

            const pixel* src = &atlas->array_pixels [index.offset + (row * atlas->width)];
            pixel*       dst = &img->data.pixels    [row * index.width];
            memcpy((void*)dst, (const void*)src, size_row);
        }

        return(img);
    }
    
    IFB_INTERNAL image*
    sprite_mngr_get_sprite_atlas_image_by_handle(
        const hnd_sprite_atlas h_atlas,
        const hnd_arena        h_arena) {

        assert(h_atlas != INVALID_HANDLE);
        assert(h_arena != INVALID_HANDLE);

        // get the atlas
        const asset_sprite_atlas* atlas = sprite_mngr_find_atlas(h_atlas);
        if (atlas == NULL) return(NULL);

        // calculate the size
        const u32 size_image_struct = sizeof(image);
        const u32 size_pixels       = atlas->width * atlas->height * sizeof(pixel);
        const u32 size_push         = size_image_struct + size_pixels;

        // allocate memory
        auto img = (image*)arena_push(h_arena, size_push);
        if (img == NULL) return(NULL);

        // set image properties
        // and copy pixel data
        img->data.pixels  = (pixel*)((addr)img + size_image_struct);
        img->data.size    = size_pixels;
        img->pixel_width  = atlas->width;
        img->pixel_height = atlas->height;
        (void)memcpy(
            (void*)img->data.vptr,
            (void*)atlas->array_pixels,
            size_pixels
        );
        return(img);
    }
    
    IFB_INTERNAL sprite_memory*
    sprite_mngr_get_memory(
        void) {

        assert(_sprite_mngr);
        assert(_sprite_mngr->mem);

        return(_sprite_mngr->mem);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // NULL if the atlas isn't loaded
    inline const asset_sprite_atlas*
    sprite_mngr_find_atlas(
        const hnd_sprite_atlas h_atlas) {

        assert(_sprite_mngr            != NULL);
        assert(_sprite_mngr->atlas_tbl != NULL);

        if (h_atlas == INVALID_HANDLE) {
            return(NULL);
        }

        auto tbl = _sprite_mngr->atlas_tbl;
        for (
            u32 i_tbl = 0;
                i_tbl < config::sprite_atlas_table_capacity;
              ++i_tbl) {

            if (tbl->array_hnd[i_tbl] == h_atlas) {
                assert(tbl->array_atlas[i_tbl]);
                return(tbl->array_atlas[i_tbl]);
            }
        }

        return(NULL);
    }

    // INVALID_INDEX if the sprite isn't in the atlas
    inline u32
    sprite_mngr_find_sprite_index(
        const asset_sprite_atlas* atlas,
        const hnd_sprite          h_sprite) {

        assert(atlas);

        if (h_sprite == INVALID_HANDLE) {
            return(INVALID_INDEX);
        }

        for (
            u32 i_sprite = 0;
                i_sprite < atlas->sprite_count;
              ++i_sprite) {

            if (atlas->array_handles[i_sprite] == h_sprite) {
                return(i_sprite);
            }
        }

        return(INVALID_INDEX);
    }
};
