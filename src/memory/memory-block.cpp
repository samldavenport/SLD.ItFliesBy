#pragma once

#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void          block_alctr_validate   (const block_allocator* alctr);
    IFB_INLINE void          block_memory_validate  (const block_memory*    mem);
    IFB_INLINE block_memory* block_memory_from_vptr (const void*            vptr);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    block_alctr_init(
        block_allocator* alctr,
        memory           mem,
        const u32        granularity) {

        const u32 size_block_header = sizeof(block_memory);

        assert(
            alctr       != NULL               &&
            mem.address != 0                  &&
            mem.size    >  size_block_header  &&
            granularity != 0
        );

        zero_memory((void*)alctr, sizeof(block_allocator));

        const u32 block_count = (mem.size / granularity);

        alctr->start             = mem.address;
        alctr->block_size        = granularity;
        alctr->block_count_total = block_count;
        alctr->block_count_free  = block_count;

        block_memory* curr = NULL;
        block_memory* prev = (block_memory*)alctr->start;
        prev->alctr = alctr;
        prev->prev  = NULL;
        prev->next  = NULL;
        prev->id    = 0;

        for (
            u32 block_id = 1;
                block_id < block_count;
              ++block_id
        ) {

            curr = (block_memory*)((addr)prev + granularity);
            curr->alctr = alctr;
            curr->prev  = prev;
            curr->next  = NULL;
            curr->id    = block_id;

            prev->next = curr;
            prev       = curr;
        }
    }

    IFB_INTERNAL void*
    block_alloc(
        block_allocator* alctr) {

        block_alctr_validate(alctr);

        block_memory* block = alctr->list.free;
        if (block) {

            assert(block->prev == NULL);
            block_memory* free_next = block->next;
            block_memory* used_next = alctr->list.used;

            // update the free list
            free_next->prev  = NULL;
            alctr->list.free = free_next;

            // update the used list
            if (used_next) {
                assert(used_next->prev == NULL);
                used_next->prev = block;
            }
            block->next = used_next;
            alctr->list.used = block;
            --alctr->block_count_free;
        }
        return(block);
    }

    IFB_INTERNAL void
    block_free(
        void* mem) {

        block_memory* block = block_memory_from_vptr(mem);
        block_memory_validate(block);

        block_allocator* alctr = block->alctr;
        block_alctr_validate(alctr);

        block_memory* used_next = block->next;
        block_memory* used_prev = block->prev;
        block_memory* free_next = alctr->list.free;

        // update the used list
        if (used_next)                 used_next->prev  = used_prev;
        if (used_prev)                 used_prev->next  = used_next;
        if (block == alctr->list.used) alctr->list.used = used_next;        

        // update the free list    
        if (free_next) {
            assert(free_next->prev == NULL);
            free_next->prev = block;
        }
        block->prev = NULL;
        block->next = free_next;
        alctr->list.free = block;
        ++alctr->block_count_free;
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    block_alctr_validate(
        const block_allocator* alctr) {

        assert(
            alctr !=                 NULL &&
            alctr->start             != 0 &&
            alctr->block_size        != 0 &&
            alctr->block_count_total != 0 && 
            (
                alctr->list.free != NULL ||
                alctr->list.used != NULL
            ) &&
            alctr->block_count_free  <= alctr->block_count_total 
        );
    }

    IFB_INLINE void
    block_memory_validate(
        const block_memory* mem) {

        assert(
            mem        != NULL &&
            mem->alctr != NULL
        );

        const block_allocator* alctr           = mem->alctr;
        const addr             alctr_mem_start = alctr->start;
        const addr             alctr_mem_end   = alctr_mem_start + (alctr->block_size * (alctr->block_count_total - 1));
        const addr             mem_addr        = (addr)mem;

        assert(
            mem->id  <  alctr->block_count_total &&
            mem_addr >= alctr_mem_start          &&
            mem_addr <= alctr_mem_end
        );
    }

    IFB_INLINE block_memory*
    block_memory_from_vptr(
        const void* vptr) {

        assert(vptr);

        const u32     block_header_size = sizeof(block_memory);
        block_memory* block             = (block_memory*)(((addr)vptr) - block_header_size); 
        return(block);
    }
};