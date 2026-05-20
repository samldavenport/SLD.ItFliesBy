#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_ENG_API_FUNC eng_context*
    eng_context_create(
        eng_mem_map* mem_map) {

        const u32 size_struct_ctx   = sizeof(eng_context);
        const u32 size_struct_stack = sizeof(eng_stack);

        zero_memory(mem_map->stack.ptr, mem_map->stack.size);

        // context
        _eng_ctx = (eng_context*)mem_map->stack.ptr;

        // stack
        auto stack      = (eng_stack*)(((byte*)_eng_ctx) + size_struct_ctx);
        stack->start    =      (byte*)(((byte*)stack)    + size_struct_stack);
        stack->size     = mem_map->stack.size - (size_struct_ctx + size_struct_stack);
        stack->position = 0;
        _eng_ctx->stack = stack;

        return(_eng_ctx);
    }

    IFB_ENG_INTERNAL byte*
    eng_stack_push_data(
        const u32 size) {

        assert(_eng_ctx && _eng_ctx->stack);

        eng_stack* stack = _eng_ctx->stack;        

        const u32 new_position = (stack->position + size);
        assert(new_position <= stack->size);

        byte* data = &stack->start[stack->position];

        stack->position = new_position;

        return(data);
    }
    
    template<typename t>
    IFB_ENG_INTERNAL t* 
    eng_stack_push_struct(
        const u32 count) {

        assert(count != 0);

        const u32 push_size = count * sizeof(t);

        t* push = eng_stack_push_data(push_size);
        assert(push);

        return(push);
    }

};