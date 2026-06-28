#pragma once

#include "ifb.hpp"

namespace ifb {

    IFB_ENGINE_API u32
    eng_image_size(
        const eng_file_handle file_hnd) {

        const u32    file_size = eng_file_size(file_hnd); 
        const cchar* file_str  = eng_file_read(file_hnd);
        if (file_str == NULL) {
            return(0);
        }

        u32 width, height, channels;
        const u32 result = stbi_info_from_memory(
            file_str,
            file_size,
            &width,
            &height,
            &channels
        );
        assert(result == 1);

        const u32 size = (width * height * channels);
        return(size); 
    }

    IFB_ENGINE_API const image*
    eng_image_load_to_arena(
        const eng_file_handle  file_hnd,
        const eng_arena_handle arena) {


    }

    IFB_ENGINE_API const image*
    eng_image_load_to_memory(
        const eng_file_handle file_hnd,
        const memory* mem) {

    }
};