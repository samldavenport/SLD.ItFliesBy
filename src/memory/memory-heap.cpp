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
         b32         is_free;
    };

    struct heap_alctr {
        heap_block* block_list;
        u32         capacity;
        u32         granularity;
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------

    inline void        heap_alctr_validate              (const heap_alctr* alctr);
    inline u32         heap_alctr_max_blocks            (const heap_alctr* alctr);
    inline void*       heap_alctr_get_memory_from_block (const heap_block* block);
    inline void*       heap_alctr_get_block_from_memory (const void*       mem);
    inline heap_block* heap_alctr_split_block           (heap_block* block, const u32 size);   
    inline u32         heap_alctr_block_size_total      (const heap_block* block);
    inline u32         heap_alctr_block_size_total      (const u32         mem_size);

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
        list->size    = alctr->capacity - sizeof(heap_block);
        list->is_free = true;
        list->next    = NULL;

        return(alctr);
    }

    IFB_INTERNAL void*
    heap_alloc(
        heap_alctr* alctr,
        const u32   size) {

        heap_alctr_validate(alctr);
        assert(size != 0);


        const bool is_pow_2     = size_is_pow_2(alctr->granularity);
        const u32  max_count    = heap_alctr_max_blocks(alctr);
        const u32  size_aligned = is_pow_2 ? size_align_pow_2(size, alctr->granularity) : size_align(size, alctr->granularity); 
        const u32  size_total   = sizeof(heap_block) + size_aligned; 
        heap_block* block       = alctr->block_list;
        void*       mem         = NULL;
        for (
            u32 block_index = 0;
            block_index < max_count;
            ++block_index) {

            // determine if the current block can accomodate the size
            const bool is_perfect_fit = block->is_free && block->size == size_total;
            const bool can_split      = block->is_free && block->size >  size_total;                 

            // if it is a perfect fit, just set it to occupied and return
            if (is_perfect_fit) {
                block->is_free = false;
                mem            = heap_alctr_get_memory_from_block(block);  
                break;
            }

            // otherwise, check if we can split this block
            if (can_split) {
                heap_block* block_split = heap_alctr_split_block(block, size_total);
                assert(block_split);
                mem = heap_alctr_get_memory_from_block(block_split);
                break;
            }
        }

        return(mem);
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

        // NOTE(SLD):
        // all we are doing here is calculating
        // a worst case scenario so loops have
        // a constant upper bound to check against

        const u32 block_size  = sizeof(heap_block) + alctr->granularity;
        const u32 block_count = alctr->capacity / block_size;

        return(block_count);
    }
    
    inline void*
    heap_alctr_get_memory_from_block(
        const heap_block* block) {

        void* mem = (void*)((addr)block + sizeof(heap_block));
        return(mem);
    }

    inline void*
    heap_alctr_get_block_from_memory(
        const void* mem) {

        heap_block* block = (heap_block*)((addr)mem - sizeof(heap_block));
        return(block);
    }

    inline heap_block*
    heap_alctr_split_block(
        heap_block* block,
        const u32   size) {

        //TODO(SLD): verify

        heap_block* block_next = block->next;  
        heap_block* block_new  = (heap_block*)((addr)block + size);  
        block_new->size    = block->size - size;
        block_new->is_free = true;
        block_new->next    = block_next;

        block->is_free = false;
        block->size    = size;
    }   

    inline u32
    heap_alctr_block_size_total(
        const heap_block* block) {

        const u32 size = sizeof(heap_block) + block->size;
        return(size);
    }

    inline u32
    heap_alctr_block_size_total(
        const u32 mem_size) {

        const u32 size = sizeof(heap_block) + mem_size;
        return(size);
    }
};
