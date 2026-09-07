#pragma once

#include "memory.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct heap_block {
         heap_block* next;
         u32         size;
         b32         free;
    };

    struct heap_alctr {
        heap_block* block_list;
        u32         capacity;
        u32         granularity;
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------

    inline void heap_alctr_validate   (const heap_alctr* alctr);
    inline u32  heap_alctr_max_blocks (const heap_alctr* alctr);

    //--------------------------------------------------------------------
    // INTERNAL DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL heap_alctr*
    heap_alctr_init(
        memory&   mem,
        const u32 granularity) {

        assert(mem.size    != 0);
        assert(mem.address != 0);
        assert(granularity != 0);

        // make sure we have enough memory
        const u32 size_alctr = sizeof(heap_alctr);
        const u32 size_min   = size_alctr * 2;
        if (mem.size < size_min) return(NULL);

        // cast pointers
        auto alctr = (heap_alctr*)mem.ptr;
        auto list  = (heap_block*)(mem.address + size_alctr);  
       
        // initialize allocator
        alctr->block_list  = list;
        alctr->capacity    = mem.size - size_alctr; 
        alctr->granularity = granularity;
   
        // initialize first node
        list->size = alctr->capacity - sizeof(heap_block);
        list->free = true;
        list->next = NULL;

        return(alctr);
    }

    IFB_INTERNAL void*
    heap_alloc(
        heap_alctr* alctr,
        const u32   size) {

        heap_alctr_validate(alctr);
        assert(size != 0);
    }

    IFB_INTERNAL void
    heap_free(
        heap_alctr* alctr,
        const void* mem) {

        heap_alctr_validate(alctr);
        assert(mem);
    }
    
    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------

    inline void
    heap_alctr_validate(
        const heap_alctr* alctr) {

        assert(alctr              != NULL);
        assert(alctr->capacity    != 0);
        assert(alctr->granularity != 0);
        assert(alctr->block_list  != NULL);
    }
    
    inline u32
    heap_alctr_max_blocks(
        const heap_alctr* alctr) {

    }
};
