#pragma once

#include "file-common.cpp"
#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "sld.hpp"
#include <cassert>
#include "eng-file.hpp"
#include "eng-memory.hpp"

namespace ifb {

    IFB_ENGINE_API u32
    eng_text_file_size(
        const hnd_file h_txt_file) {

        assert(h_txt_file != FILE_HANDLE_INVALID);

        const u32 size = file_get_size(h_txt_file); 
        return(size);
    }

    IFB_ENGINE_API const text*
    eng_text_file_load_to_arena(
        const hnd_file  h_txt_file,
        const hnd_arena h_arena) {

        assert(h_txt_file != FILE_HANDLE_INVALID);
        assert(h_arena    != INVALID_HANDLE);
   
        const u32 size_file = file_get_size(h_txt_file);
        const u32 size_push = sizeof(text) + size_file + 1; 
     
        auto txt = (text*)arena_push(h_arena, size_push);
        if (txt) {

            zero_memory((void*)txt, size_push);

            txt->size = size_file;
            txt->cstr = ((char*)txt) + sizeof(text);

            const cchar* file_buf = file_read(h_txt_file, size_file);
            (void)memmove(
                (void*)txt->cstr,
                (void*)file_buf,
                size_file
            );
        }
        return(txt);
    }

    IFB_ENGINE_API const text*
    eng_text_file_load_to_memory(
        const hnd_file h_txt_file,
        memory&        mem) {

        assert(h_txt_file  != FILE_HANDLE_INVALID);
        assert(mem.size    != 0);
        assert(mem.address != 0);

        zero_memory(mem);

        const u32 size_file = file_get_size(h_txt_file);
        const u32 size_read = mem.size + sizeof(text) < size_file ? mem.size : size_file; 
    
        auto txt = (text*)mem.address;
        txt->size = size_file;
        txt->cstr = (char*)mem.address + sizeof(text);

        const cchar* file_buf = file_read(h_txt_file, size_file);
        (void)memmove(
            (void*)txt->cstr,
            (void*)file_buf,
            size_file
        );
        return(txt);
    }
};
