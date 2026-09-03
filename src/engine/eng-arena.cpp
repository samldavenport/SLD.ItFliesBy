#pragma once

#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    IFB_ENGINE_API eng_arena_handle
    eng_arena_alloc(
        void) {

        const eng_arena_handle hnd = arena_alloc();
        return(hnd);        
    }

    IFB_ENGINE_API void
    eng_arena_free(
        const eng_arena_handle hnd) {

        arena_free(hnd);
    }

    IFB_ENGINE_API void
    eng_arena_reset(
        const eng_arena_handle hnd) {

        arena_reset(hnd);
    }

    IFB_ENGINE_API u32
    eng_arena_save(
        const eng_arena_handle hnd) {

        return(arena_save(hnd));
    }

    IFB_ENGINE_API u32
    eng_arena_size_free(
        const eng_arena_handle hnd) {

        return(arena_size_free(hnd));
    }

    IFB_ENGINE_API u32
    eng_arena_size_used(
        const eng_arena_handle hnd) {

        return(arena_size_used(hnd));
    }

    IFB_ENGINE_API void
    eng_arena_revert(
        const eng_arena_handle hnd,
        const u32              save) {

        arena_revert(hnd, save);
    }
    
    IFB_ENGINE_API void*
    eng_arena_push(
        const eng_arena_handle hnd,
        const u32              size) {

        return(arena_push(hnd, size));
    }
};
