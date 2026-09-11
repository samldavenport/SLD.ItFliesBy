#pragma once

#include "ifb-collections.hpp"
#include "ifb-types.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STACK 
    //--------------------------------------------------------------------
    
    struct stack {
        addr start;
        u32  capacity;
        u32  position;
        u32  save;
    };

    //--------------------------------------------------------------------
    // INLINE METHODS 
    //--------------------------------------------------------------------
   
    inline void
    stack_validate(
        const stack* s) {

        assert(s);
        assert(s->start    != 0);
        assert(s->capacity != 0);
        assert(s->position <= s->capacity);
        assert(s->save     <= s->position);
    }

    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    u32
    stack_memory_requirement(
        const u32 capacity) {

        const u32 size_struct = sizeof(stack);
        const u32 size_req    = size_struct + capacity;
        return(size_req);
    }

    stack*
    stack_memory_create(
        const u32 capacity,
        memory&   mem) {

        assert(capacity    != 0);
        assert(mem.size    != 0);
        assert(mem.address != 0);
    
        const u32 size_min = stack_memory_requirement(capacity);

        stack* s = (stack*)mem.ptr;
        s->start    = mem.address + sizeof(stack);
        s->capacity = capacity;
        s->position = 0;
        s->save     = 0;
    
        return(s);
    }

    stack* 
    stack_arena_create(
        const u32          capacity,
        const arena_handle arena_hnd) {
   
        assert(capacity  != 0);
        assert(arena_hnd != INVALID_HANDLE);

        memory mem;
        mem.size = stack_memory_requirement(capacity);
        mem.ptr  = eng_arena_push(arena_hnd, mem.size);
    
        stack* s = stack_memory_create(capacity, mem); 
        assert(s);

        return(s);
    }

    u32
    stack_size_get_capacity(
        const stack* s) {

        stack_validate(s);
        return(s->capacity);
    }

    u32
    stack_get_position(
        const stack* s) {

        stack_validate(s);
        return(s->position);
    }

    void*
    stack_get_head(
        const stack* s) {

        stack_validate(s);
        return((void*)s->start);
    }

    void*
    stack_get_tail(
        const stack* s) {

        stack_validate(s);
        return((void*)(s->start + s->position));
    }
    
    //--------------------------------------------------------------------
    // PUBLIC METHODS
    //--------------------------------------------------------------------

    u32 
    stack_save(stack* s) {

        stack_validate(s);

        assert(s->save == 0);
        s->save = s->position;
       
        return(s->save);
    }

    void 
    stack_reset(
        stack* s) {

        stack_validate(s);
        s->position = 0;
        s->save     = 0;
    }

    void* 
    stack_push(
        stack* s,
        const u32 size) {

        stack_validate(s);

        void* push_mem = NULL;

        const u32 new_position = (s->position + size);
        if (new_position <= s->capacity) {

            push_mem = (void*)(s->start + s->position);
            s->position = new_position;
        }

        return(push_mem);
    }

    void
    stack_pull(
        stack*    s,
        const u32 size) {

        stack_validate(s);

        if (size <= s->position) {
            s->position -= size;
        }
    }

    void
    stack_revert(
        stack* s,
        const u32 save) {

        stack_validate(s);

        assert(s->save == save);
        s->position  = s->save;
        s->save      = 0;
    }
};
