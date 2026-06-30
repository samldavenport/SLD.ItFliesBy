#pragma once

#include "ifb.hpp"

namespace ifb {

    IFB_ENGINE_API u32
    eng_image_size(
        const eng_file_handle file_hnd) {

        const u32     file_size = eng_file_get_size (file_hnd); 
        const cchar* file_str  = eng_file_read     (file_hnd);
        if (file_str == NULL) {
            return(0);
        }

        s32 width, height, channels;
        const u32 result = stbi_info_from_memory(
            (const stbi_uc*)file_str,
            file_size,
            &width,
            &height,
            &channels
        );
        assert(result == 1);

        const u32 size = (width * height * channels) + sizeof(image);
        return(size); 
    }

    IFB_ENGINE_API const image*
    eng_image_load_to_arena(
        const eng_file_handle  file_hnd,
        const eng_arena_handle arena) {

        // get image size
        const u32 img_size = eng_image_size(file_hnd);
        if (img_size == 0) {
            return(NULL);
        }

        // allocate image memory
        const u32 push_size = img_size;
        void*     img_mem   = eng_arena_push(arena, push_size); 
        assert(img_mem != NULL);

        // init image
        image* img         = (image*)img_mem;
        img->data.address  = ((addr)img) + sizeof(image);
        img->data.size     = push_size - sizeof(image); 
        img->pixel_width   = 0;
        img->pixel_height  = 0;
        img->pixel_count   = 0;

        // get image file buffer
        const stbi_uc* img_file_buffer_ptr  = (const stbi_uc*)eng_file_read(file_hnd);
        const u32      img_file_buffer_size = eng_file_get_size(file_hnd);
        assert(img_file_buffer_ptr != NULL);

        // load the image
        s32 channels;
        stbi_uc* tmp_img_data = stbi_load_from_memory(
            img_file_buffer_ptr,
            img_file_buffer_size,
            (s32*)&img->pixel_width,
            (s32*)&img->pixel_height,
            &channels,
            4
        );

        // copy the image to our buffer
        (void*)memmove(
            img->data.vptr,
            tmp_img_data,
            img->data.size
        );

        // free the loaded image data and return
        stbi_image_free(tmp_img_data);
        return(img);
    }

    IFB_ENGINE_API const image*
    eng_image_load_to_memory(
        const eng_file_handle file_hnd,
        const memory&         mem) {

        // get image size
        const u32 img_size = eng_image_size(file_hnd);
        if (img_size == 0) {
            return(NULL);
        }

        // check memory requirements
        assert(mem.address != 0 && mem.size == img_size);

        // init image
        image* img         = (image*)mem.ptr;
        img->data.address  = ((addr)img) + sizeof(image);
        img->data.size     = mem.size - sizeof(image); 
        img->pixel_width   = 0;
        img->pixel_height  = 0;
        img->pixel_count   = 0;

        // get image file buffer
        const stbi_uc* img_file_buffer_ptr  = (const stbi_uc*)eng_file_read(file_hnd);
        const u32      img_file_buffer_size = eng_file_get_size(file_hnd);
        assert(img_file_buffer_ptr != NULL);

        // load the image
        s32 channels;
        stbi_uc* tmp_img_data = stbi_load_from_memory(
            img_file_buffer_ptr,
            img_file_buffer_size,
            (s32*)&img->pixel_width,
            (s32*)&img->pixel_height,
            &channels,
            4
        );

        // copy the image to our buffer
        (void*)memmove(
            img->data.vptr,
            tmp_img_data,
            img->data.size
        );

        // free the loaded image data and return
        stbi_image_free(tmp_img_data);
        return(img);
    }
};