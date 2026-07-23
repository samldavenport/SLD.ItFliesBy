#pragma once

#include "memory.hpp"

namespace ifb {

    IFB_INLINE void block_alctr_validate(const block_allocator* alctr);

    IFB_INTERNAL u32
    block_alctr_mem_requriement(
        const u32 granularity,
        const u32 block_count) {

        assert(granularity != NULL);
        assert(block_count != NULL);

        const u32 size_block = granularity + sizeof(block_memory);
        const u32 size_req   = (block_count <= 64)
            ? (size_block * block_count)
            : (size_block * 64);

        return(size_req);
    }

    IFB_INTERNAL void
    block_alctr_init(
        block_allocator* alctr,
        memory           mem,
        const u32        granularity) {

        const u32 block_size  = (granularity + sizeof(block_memory)); 
        const u32 block_count = (mem.size / block_size);

        assert (alctr       != NULL);
        assert (mem.ptr     != NULL);
        assert (mem.size    != 0);
        assert (granularity != 0);
        assert (granularity < mem.size);
        assert (block_size  < mem.size);
        assert (block_count > 1);

        alctr->block_count_total = (block_count <= 64) ? block_count : 64;         
        alctr->block_mask_hi     = 0;
        alctr->block_mask_lo     = 0;
        alctr->block_size        = block_size;
        alctr->start             = mem.address;
    }

    IFB_INTERNAL void*
    block_alloc(
        block_allocator* alctr) {

        // validate allocator
        block_alctr_validate(alctr);

        // get low block count
        const u32 block_count_lo = (alctr->block_count_total <= 32)
            ? alctr->block_count_total
            : 32;

        // get high block count
        const u32 block_count_hi = (alctr->block_count_total > 32)
            ? alctr->block_count_total - 32
            : 0;

        // look for a free low block
        assert(block_count_lo != 0);
        u32 free_block = INVALID_INDEX;
        for (
            u32 block = 0;
                block < block_count_lo;
              ++block) {

            const bool is_free = ((alctr->block_mask_lo >> block) & 1) == 0;
            if (is_free) {
                free_block = block;
                break;                
            }
        }

        // if we didn't find a free low block
        // and we have 0 high blocks,
        // we are done 
        if (free_block == INVALID_INDEX && alctr->block_count_total <= 32) {
            assert(block_count_hi == 0);
            return(NULL);
        }

        // if we don't have a free block
        // and we have high blocks, search them
        assert(block_count_lo == 32 && alctr->block_count_total > 32);
        assert(block_count_hi != 0);
        if (free_block == INVALID_INDEX && block_count_hi > 0) {
            for (
                u32 block = 0;
                    block < block_count_hi;
                  ++block) {

                const bool is_free = ((alctr->block_mask_hi >> block) & 1) == 0;
                if (is_free) {
                    free_block = (block + 32);
                    break;                
                }
            }
        }

        // if we still don't have a free block,
        // we have no free blocks
        if (free_block == INVALID_INDEX) {
            return(NULL);
        }

        // we have a free block
        // calculate the start of the block header
        const addr block_start = alctr->start + (free_block * alctr->block_size);
        auto*      block_mem   = (block_memory*)block_start;

        // cast the block header
        block_mem->alctr = alctr;
        block_mem->id    = free_block;

        // toggle this block used
        if (block_mem->id < 32) alctr->block_mask_lo |= (1 << block_mem->id);
        else                    alctr->block_mask_hi |= (1 << (block_mem->id - 32));

        // calculate where the user memory starts
        // and return a vptr
        void* mem = (void*)(block_start + sizeof(block_memory)); 
        return(mem);    
    }

    IFB_INTERNAL void
    block_free(
        void* mem) {

        assert(mem != NULL);

        // get the block memory
        const auto mem_start = (addr)mem;
        auto       block_mem = (block_memory*)(mem_start - sizeof(block_memory)); 
        
        // validate the block memory
        assert(block_mem->id    < 64);
        assert(block_mem->alctr != NULL);

        // get the allocator
        block_allocator* alctr          = block_mem->alctr;
        const addr       block_max_addr = (alctr->start + (alctr->block_size * alctr->block_count_total));

        // validate the allocator
        assert(mem_start <= block_max_addr);
        assert(mem_start >= alctr->start);
        block_alctr_validate(alctr);
        
        // validate the id of the block
        const addr block_offset = ((addr)block_mem - alctr->start);
        assert(block_mem->id % alctr->block_size == 0);
        const u32 block_id = (block_offset / alctr->block_size);
        assert(block_id == block_mem->id);

        // toggle the block id as free
        if (block_id < 32) {
            assert((alctr->block_mask_lo >> block_id) & 1 == 1);
            alctr->block_mask_lo &= ~(1 << block_id);
        }
        else {
            const u32 block_id_hi = (block_id - 32);
            assert((alctr->block_mask_hi >> block_id_hi) & 1 == 1);
            alctr->block_mask_hi &= ~(1 << block_id_hi);
        }
    }
};

