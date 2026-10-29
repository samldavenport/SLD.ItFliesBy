#pragma once

#include "eng-sprite.cpp"
#include "file-ro.cpp"
#include "file-rw.cpp"
#include "ifb.hpp"
#include "eng-sprite.hpp"
#include "eng-file.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // HEADER FORMAT
    //--------------------------------------------------------------------
    // - verification string    | 14 bytes
    // - asset count (n)        | 4 bytes
    // - atlas pixel width  (w) | 4 bytes
    // - atlas pixel height (h) | 4 bytes
    // - handles                | n * 4 bytes
    // - names                  | n * IFB_CONFIG_ASSET_NAME_SIZE bytes
    // - indexes                | n * 12 bytes
    //   - width                | 4 bytes
    //   - height               | 4 bytes
    //   - offset               | 4 bytes (pixel index into the atlas, y * w + x)
    //--------------------------------------------------------------------
    // IMAGE FORMAT
    //--------------------------------------------------------------------
    // - pixels (rgba)          | w * h * 4 bytes
    //--------------------------------------------------------------------
    
    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    const cchar SPRITE_ATLAS_VERIFICATION_CSTR[] = "IFBSPRITEATLAS";

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

   
    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------
    
    inline u32  sprite_atlas_file_write_at (const hnd_file h_file, const u32 offset, const u32 size, const void* data);
    inline bool sprite_atlas_file_read_at  (const hnd_file h_file, const u32 offset, const u32 size, void*       data);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS 
    //--------------------------------------------------------------------

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

            const hnd_asset   h_img = sprite_list_get_img_asset_hnd(node);
            const asset_name* name  = asset_mngr_get_name(h_img);
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

    // returns false if the file can't be opened or a write comes up short
    IFB_INTERNAL bool
    sprite_atlas_save_to_file(
        sprite_atlas* atlas,
        const cchar*  path) {

        assert(atlas);
        assert(path);

        // the atlas has to be built first
        const image* img   = atlas->img;
        const u32    count = atlas->header.sprite_count;
        assert(img   != NULL);
        assert(count != 0);

        // open the file
        const hnd_file h_file = file_rw_overwrite(path);
        if (h_file == FILE_HANDLE_INVALID) return(false);

        // calculate offsets
        const u32 size_verification = sizeof(SPRITE_ATLAS_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_width      = offset_count   + sizeof(u32);
        const u32 offset_height     = offset_width   + sizeof(u32);
        const u32 offset_handles    = offset_height  + sizeof(u32);
        const u32 offset_names      = offset_handles + (count * sizeof(hnd_sprite));
        const u32 offset_indexes    = offset_names   + (count * sizeof(asset_name));
        const u32 offset_pixels     = offset_indexes + (count * sizeof(sprite_index));
        const u32 file_size         = offset_pixels  + img->data.size;

        // keep track of how much we write
        u32 bytes_written = 0;

        // write everything up to and excluding the handles
        bytes_written += sprite_atlas_file_write_at(h_file, 0,             size_verification, (void*)SPRITE_ATLAS_VERIFICATION_CSTR);
        bytes_written += sprite_atlas_file_write_at(h_file, offset_count,  sizeof(u32),       (void*)&count);
        bytes_written += sprite_atlas_file_write_at(h_file, offset_width,  sizeof(u32),       (void*)&img->pixel_width);
        bytes_written += sprite_atlas_file_write_at(h_file, offset_height, sizeof(u32),       (void*)&img->pixel_height);

        // handles and names are contiguous, so they go in one write each
        bytes_written += sprite_atlas_file_write_at(h_file, offset_handles, count * sizeof(hnd_sprite), (void*)atlas->header.array_handles);
        bytes_written += sprite_atlas_file_write_at(h_file, offset_names,   count * sizeof(asset_name), (void*)atlas->header.array_name);

        // write indexes
        sprite_index index = {0};
        for (
            u32 i_sprite = 0;
                i_sprite < count;
              ++i_sprite) {

            const u32            offset_index = offset_indexes + (i_sprite * sizeof(sprite_index));
            const dimensions_2d& dims         = atlas->header.array_dims[i_sprite];

            index.width  = (u32)dims.width;
            index.height = (u32)dims.height;
            index.offset = atlas->header.array_offset[i_sprite];

            bytes_written += sprite_atlas_file_write_at(
               h_file,
               offset_index,
               sizeof(sprite_index),
               (void*)&index
            );
        }

        // write pixels
        bytes_written += sprite_atlas_file_write_at(
            h_file,
            offset_pixels,
            img->data.size,
            img->data.vptr
        );

        file_close(h_file);
        return(bytes_written == file_size);
    }

    // returns false if the file can't be opened, isn't an atlas, doesn't match its header, or there's no room on the block
    IFB_INTERNAL bool
    sprite_atlas_load_from_file(
        sprite_atlas* atlas,
        const cchar*  path) {

        assert(atlas);
        assert(path);
        assert(atlas->header.sprite_count == 0);

        // open the file
        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) return(false);

        // the file needs to be at least big enough for everything up to the handles
        const u32 size_file         = file_get_size(h_file);
        const u32 size_verification = sizeof(SPRITE_ATLAS_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_width      = offset_count  + sizeof(u32);
        const u32 offset_height     = offset_width  + sizeof(u32);
        const u32 offset_handles    = offset_height + sizeof(u32);
        if (size_file < offset_handles) {
            file_close(h_file);
            return(false);
        }

        // check the verification string
        cchar verification[sizeof(SPRITE_ATLAS_VERIFICATION_CSTR) - 1];
        bool is_valid = sprite_atlas_file_read_at(h_file, 0, size_verification, verification);
        is_valid     &= (memcmp(verification, SPRITE_ATLAS_VERIFICATION_CSTR, size_verification) == 0);

        // read the count and atlas size
        u32 count        = 0;
        u32 pixel_width  = 0;
        u32 pixel_height = 0;
        is_valid &= sprite_atlas_file_read_at(h_file, offset_count,  sizeof(u32), &count);
        is_valid &= sprite_atlas_file_read_at(h_file, offset_width,  sizeof(u32), &pixel_width);
        is_valid &= sprite_atlas_file_read_at(h_file, offset_height, sizeof(u32), &pixel_height);
        is_valid &= (count != 0 && pixel_width != 0 && pixel_height != 0);
        if (!is_valid) {
            file_close(h_file);
            return(false);
        }

        // header offsets, 64 bits so a garbage header can't overflow
        const u64 pixel_count    = (u64)pixel_width * pixel_height;
        const u64 offset_names   = offset_handles + ((u64)sizeof(hnd_sprite)   * count);
        const u64 offset_indexes = offset_names   + ((u64)sizeof(asset_name)   * count);
        const u64 offset_pixels  = offset_indexes + ((u64)sizeof(sprite_index) * count);
        const u64 size_pixels    = pixel_count * sizeof(pixel);
        if (offset_pixels + size_pixels != size_file) {
            file_close(h_file);
            return(false);
        }

        // allocate everything on the atlas block
        auto img           =         (image*)sprite_memory_block_push(atlas->mem_block, sizeof(image));
        auto pixels        =         (pixel*)sprite_memory_block_push(atlas->mem_block, (u32)size_pixels);
        auto array_handles =    (hnd_sprite*)sprite_memory_block_push(atlas->mem_block, count * sizeof(hnd_sprite));
        auto array_name    =    (asset_name*)sprite_memory_block_push(atlas->mem_block, count * sizeof(asset_name));
        auto array_dims    = (dimensions_2d*)sprite_memory_block_push(atlas->mem_block, count * sizeof(dimensions_2d));
        auto array_offset  =           (u32*)sprite_memory_block_push(atlas->mem_block, count * sizeof(u32));
        if (!img || !pixels || !array_handles || !array_name || !array_dims || !array_offset) {
            file_close(h_file);
            return(false);
        }

        // handles, names, and pixels are contiguous, so they go in one read each
        is_valid &= sprite_atlas_file_read_at(h_file, offset_handles,      count * sizeof(hnd_sprite), array_handles);
        is_valid &= sprite_atlas_file_read_at(h_file, (u32)offset_names,   count * sizeof(asset_name), array_name);
        is_valid &= sprite_atlas_file_read_at(h_file, (u32)offset_pixels,  (u32)size_pixels,           pixels);

        // indexes are split into dims and offsets
        sprite_index index = {0};
        for (
            u32 i_sprite = 0;
                i_sprite < count && is_valid;
              ++i_sprite) {

            const u32 offset_index = (u32)offset_indexes + (i_sprite * sizeof(sprite_index));
            is_valid &= sprite_atlas_file_read_at(h_file, offset_index, sizeof(sprite_index), &index);

            // the sprite has to fit inside the atlas
            const u32 x = index.offset % pixel_width;
            const u32 y = index.offset / pixel_width;
            is_valid &= (index.offset < pixel_count);
            is_valid &= ((u64)x + index.width  <= pixel_width);
            is_valid &= ((u64)y + index.height <= pixel_height);

            array_dims   [i_sprite].width  = (f32)index.width;
            array_dims   [i_sprite].height = (f32)index.height;
            array_offset [i_sprite]        = index.offset;
        }

        file_close(h_file);
        if (!is_valid) return(false);

        img->data.pixels  = pixels;
        img->data.size    = (u32)size_pixels;
        img->pixel_width  = pixel_width;
        img->pixel_height = pixel_height;

        // nothing is set until everything is read, so a failed load leaves the atlas empty
        atlas->img                  = img;
        atlas->header.array_handles = array_handles;
        atlas->header.array_name    = array_name;
        atlas->header.array_dims    = array_dims;
        atlas->header.array_offset  = array_offset;
        atlas->header.sprite_count  = count;

        return(true);
    }

    IFB_INTERNAL sprite_atlas*           sprite_atlas_load_from_asset_db         (sprite_atlas* atlas, const cchar* name);
    IFB_INTERNAL u32
    sprite_atlas_get_sprite_count(
        const sprite_atlas* atlas) {

        assert(atlas);

        return(atlas->header.sprite_count);
    }

    // null until the atlas is built or loaded
    IFB_INTERNAL const image*
    sprite_atlas_get_image(
        const sprite_atlas* atlas) {

        assert(atlas);

        return(atlas->img);
    }

    // the offset is a pixel index, so it's split back into x and y
    IFB_INTERNAL bool
    sprite_atlas_get_sprite_info(
        const sprite_atlas*       atlas,
        const u32                 index,
        sprite_atlas_sprite_info& info) {

        assert(atlas);

        if (index >= atlas->header.sprite_count) return(false);
        assert(atlas->img);

        const u32            atlas_width = atlas->img->pixel_width;
        const u32            offset      = atlas->header.array_offset [index];
        const dimensions_2d& dims        = atlas->header.array_dims   [index];

        memcpy(info.name, atlas->header.array_name[index].cstr, IFB_CONFIG_ASSET_NAME_SIZE);
        info.name[IFB_CONFIG_ASSET_NAME_SIZE] = 0;

        info.hnd    = atlas->header.array_handles[index];
        info.x      = offset % atlas_width;
        info.y      = offset / atlas_width;
        info.width  = (u32)dims.width;
        info.height = (u32)dims.height;
        return(true);
    }

    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_name         (const sprite_atlas* atlas, const cchar*     name);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_handle       (const sprite_atlas* atlas, const hnd_sprite hnd);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_index        (const sprite_atlas* atlas, const u32        index);

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS 
    //--------------------------------------------------------------------
    
    inline u32
    sprite_atlas_file_write_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        const void*    data) {

        // file_write doesn't move the cursor, so we always set it first
        file_set_cursor(h_file, offset);
        const u32 bytes_written = file_write(h_file, size, (const byte*)data);
        assert(bytes_written == size);

        return(bytes_written);
    }

    inline bool
    sprite_atlas_file_read_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        void*          data) {

        // like writing, the cursor doesn't move on its own
        file_set_cursor(h_file, offset);
        const u32  bytes_read = file_read_to_buffer(h_file, size, (byte*)data);
        const bool did_read   = (bytes_read == size);

        return(did_read);
    }
};
