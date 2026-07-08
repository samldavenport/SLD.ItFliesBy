#pragma once

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    IFB_ENGINE_API eng_arena_handle
    eng_arena_alloc(
        void) {

        const arena* a = memory_mngr_arena_alloc();
        assert(a);

        eng_arena_handle hnd = a->id();
        
        return(hnd);        
    }

    IFB_ENGINE_API void
    eng_arena_free(
        const eng_arena_handle hnd) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        memory_mngr_arena_free(&a);
        a = NULL;
    }

    IFB_ENGINE_API void
    eng_arena_reset(
        const eng_arena_handle hnd) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        a->reset();
    }

    IFB_ENGINE_API u32
    eng_arena_save(
        const eng_arena_handle hnd) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        const u32 save = a->save(); 
        return(save);
    }

    IFB_ENGINE_API u32
    eng_arena_size_free(
        const eng_arena_handle hnd) {

        //TOOD
        assert(false);

        // arena* a = memory_mngr_arena_from_handle(hnd); 
        // assert(a);

        // const u32 size_free = 
        // return(size_free);
    }

    IFB_ENGINE_API u32
    eng_arena_size_used(
        const eng_arena_handle hnd) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        return(a->size_used());
    }

    IFB_ENGINE_API void
    eng_arena_revert(
        const eng_arena_handle hnd,
        const u32              save) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        a->revert(save);
    }
    
    IFB_ENGINE_API void*
    eng_arena_push(
        const eng_arena_handle hnd,
        const u32              size) {

        arena* a = memory_mngr_arena_from_handle(hnd); 
        assert(a);

        void* mem = a->push(size);
        return(mem);
    }
};