#pragma once

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    IFB_ENGINE_API eng_arena_handle
    eng_arena_alloc(
        void) {

        const arena* a = arena_alloc();
        assert(a);

        eng_arena_handle hnd = a->id;
        
        return(hnd);        
    }

    IFB_ENGINE_API void
    eng_arena_free(
        const eng_arena_handle hnd) {

        arena* a = arena_from_handle(hnd);
        assert(a);

        arena_free(a);
    }

    IFB_ENGINE_API void
    eng_arena_reset(
        const eng_arena_handle hnd) {

        arena* a = arena_from_handle(hnd);
        assert(a);

        arena_reset(a);
    }

    IFB_ENGINE_API u32
    eng_arena_save(
        const eng_arena_handle hnd) {

        arena* a = arena_from_handle(hnd);
        assert(a);

        const u32 save = arena_save(a);
        return(save);
    }

    IFB_ENGINE_API void
    eng_arena_revert(
        const eng_arena_handle hnd,
        const u32              save) {

        arena* a = arena_from_handle(hnd);
        assert(a);

        arena_revert(a, save);
    }
    
    IFB_ENGINE_API void*
    eng_arena_push(
        const eng_arena_handle hnd,
        const u32              size) {

        arena* a = arena_from_handle(hnd);
        assert(a);

        void* mem = arena_push(a, size);
        return(mem);
    }
};