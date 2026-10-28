#pragma once

#include "eng-internal.hpp"

namespace ifb {

    IFB_ENG_INTERNAL void
    global_stack_create_and_init(
        const eng_mem_map* mem_map) {

        assert(
            mem_map             != NULL &&
            mem_map->stack.ptr  != NULL &&
            mem_map->stack.size != 0
        );

        zero_memory(mem_map->stack.ptr, mem_map->stack.size);

        _global_stack           = (global_stack*)mem_map->stack.ptr;
        _global_stack->position = sizeof(global_stack);
        _global_stack->size     = mem_map->stack.size;
    }

    IFB_ENG_INTERNAL void
    global_stack_validate(
        void) {

        assert(
            _global_stack           != NULL &&
            _global_stack->size     != 0    &&
            _global_stack->position <= _global_stack->size
        );      
    }

    IFB_ENG_INTERNAL void*
    global_alloc(
        const u32 size) {

        global_stack_validate();

        const u32 new_position = (_global_stack->position + size);
        assert(new_position <= _global_stack->size);

        addr start = (addr)_global_stack;
        addr data  = (start + _global_stack->position); 

        _global_stack->position = new_position;

        return((void*)data);
    }

    template<typename t>
    IFB_ENG_INTERNAL t*
    global_alloc(const u32 count) {
    
        const u32 size = count * sizeof(t);
        auto mem = (t*)global_alloc(size);
        assert(mem);
        return(mem);    
    }
};