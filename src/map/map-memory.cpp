#pragma once

#include "eng-internal.hpp"
#include "ifb-collections.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "map-internal.hpp"
#include "map.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------

    inline u32  map_memory_stack_capacity (map_memory_stack_list* stack_list);
    inline addr map_memory_stack_start    (map_memory_stack*      stack);
    inline addr map_memory_stack_push     (map_memory* map_mem, map_memory_stack*      stack, const u32 size);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL map_memory*
    map_memory_create(
        void) {
   
        auto map_mem    = global_alloc<map_memory>();
        auto stack_list = global_alloc<map_memory_stack_list>();

        assert(map_mem    != NULL);
        assert(stack_list != NULL);
       
        map_mem->address    = 0;
        map_mem->size       = 0;
        map_mem->stack_list = stack_list;

        return(map_mem);
    }
    
    IFB_INTERNAL void
    map_memory_init(
        map_memory*   map_mem,
        const memory& res) {

        assert(map_mem      != NULL);
        assert(res.size     != 0);
        assert(res.address  != 0);

        const auto& cfg = config_instance();

        auto stack_list = map_mem->stack_list;
        assert(stack_list != NULL);

        // initialize the stack list
        stack_list->stack_count = cfg.map_capacity;
        stack_list->stack_size  = cfg.map_stack_size;
        assert(stack_list->stack_count != 0);
        assert(stack_list->stack_size  != 0);
   
        // make sure we have enough memory
        const u32 size_min = stack_list->stack_count * stack_list->stack_size; 
        assert(res.size >= size_min);

        // commit stack memory
        map_mem->size = size_min;
        map_mem->ptr  = pfm_memory_commit(res.ptr, 0, size_min); 
        assert(map_mem->ptr != NULL);
  
        // initialize the stacks
        map_memory_stack* prev = (map_memory_stack*)map_mem->ptr;
        map_memory_stack* curr = NULL;
        for (
            u32 stack_index = 0;
            stack_index < stack_list->stack_count;
            ++stack_index) {

            curr       = (map_memory_stack*)((addr)prev + stack_list->stack_size); 
            curr->prev = prev;
            curr->next = NULL; 
            curr->pos  = 0;

            prev->next = curr;
        }
    } 
    
    IFB_INTERNAL map_memory_stack*
    map_memory_stack_alloc(
        map_memory* map_mem) {

        assert(map_mem != NULL);

        auto stack_list = map_mem->stack_list;
        assert(stack_list);
        
        map_memory_stack* new_stack = stack_list->free;
        if (!new_stack) return(NULL);

        map_memory_stack* next_used = stack_list->used;

        new_stack->prev = NULL;
        new_stack->next = next_used;
        new_stack->pos  = 0;

        next_used->prev  = new_stack;
        stack_list->used = next_used;

        return(next_used);
    }  

    IFB_INTERNAL void
    map_memory_stack_free(
        map_memory*       map_mem,
        map_memory_stack* stack) {

        assert(map_mem != NULL);
        assert(stack   != NULL);

        map_memory_stack_list* stack_list = map_mem->stack_list;
        assert(stack_list);

        map_memory_stack* next_used = stack->next;
        map_memory_stack* prev_used = stack->prev;
        map_memory_stack* next_free = stack_list->free; 

        if (next_used)                 next_used->prev  = prev_used;  
        if (prev_used)                 prev_used->next  = next_used;
        if (next_free)                 next_free->prev  = stack;
        if (stack_list->used == stack) stack_list->used = next_used;

        stack->next      = next_free;
        stack_list->free = stack;
    }

    IFB_INTERNAL map*
    map_memory_stack_push_map(
        map_memory*       map_mem,
        map_memory_stack* stack) {

        assert(map_mem != NULL);
        assert(stack   != NULL);        

        auto m = (map*)map_memory_stack_push(map_mem, stack, sizeof(map_memory));
        if (m != NULL) {
            m->stack       = stack;
            m->first_chunk = NULL;
        }

        return(m);
    }

    IFB_INTERNAL map_chunk*
    map_memory_stack_push_chunk(
        map_memory*       map_mem,
        map_memory_stack* stack) {

        assert(map_mem != NULL);
        assert(stack   != NULL);        

        auto chunk = (map_chunk*)map_memory_stack_push(map_mem, stack, sizeof(map_chunk));
        if (chunk != NULL) {
            *chunk = {0};
        }

        return(chunk);
    }
    
    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS 
    //--------------------------------------------------------------------
    
    inline u32
    map_memory_stack_capacity(
        map_memory_stack_list* stack_list) {

        const u32 capacity = stack_list->stack_size - sizeof(map_memory_stack);
        return(capacity);
    }

    inline addr
    map_memory_stack_start(
        map_memory_stack* stack) {

        addr start = (addr)((addr)stack + sizeof(map_memory_stack));
        return(start);
    }
    
    inline addr
    map_memory_stack_push(
        map_memory*       map_mem,
        map_memory_stack* stack,
        const u32         size) {

        const u32  capacity = map_memory_stack_capacity (map_mem->stack_list);
        const addr start    = map_memory_stack_start    (stack); 
    
        assert(stack->pos < capacity); 

        const u32 pos_new = stack->pos + sizeof(map);
        if (pos_new > capacity) return(NULL);

        addr push = (start + stack->pos);
        stack->pos = pos_new;

        return(push);
    }
};
