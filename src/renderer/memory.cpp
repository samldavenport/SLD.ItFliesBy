#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL void*
    renderer_memory_commit(
        renderer_context* ctx) {

        assert(ctx);

        auto& mem = ctx->mem;
        assert(
            mem.ptr                    != NULL                         &&
            mem.size                   != 0                            &&
            mem.granularity            != 0                            &&
            mem.block_stack.ids        != 0                            &&
            mem.block_stack.capacity   == (mem.size / mem.granularity) &&
            mem.block_stack.position   >  0                            &&
            mem.size % mem.granularity == 0
        );

        const u32 index  = --mem.block_stack.position;
        const u32 block  = mem.block_stack.ids[index];
        const u32 offset = block * mem.granularity;
        void*     commit = pfm_memory_commit(mem.ptr, offset, mem.granularity);
        assert(commit != 0);

        return(commit);
    
    }

    IFB_INTERNAL void
    renderer_memory_decommit(
        renderer_context* ctx, void* mem) {

        assert(ctx != NULL && mem != NULL);

        auto& rnd_mem = ctx->mem;
        assert(
            rnd_mem.ptr                        != NULL                                 &&
            rnd_mem.size                       != 0                                    &&
            rnd_mem.granularity                != 0                                    &&
            rnd_mem.block_stack.ids            != 0                                    &&
            rnd_mem.block_stack.capacity       == (rnd_mem.size / rnd_mem.granularity) &&
            rnd_mem.block_stack.position       <  rnd_mem.block_stack.capacity         &&
            rnd_mem.size % rnd_mem.granularity == 0
        );

        auto& stack = ctx->mem.block_stack;

        const addr mem_addr = (addr)mem;
        const addr offset   = (mem_addr - rnd_mem.address);
        const u32  block    = (offset   / rnd_mem.granularity);
        assert(
            rnd_mem.granularity % offset == 0 &&
            block < stack.capacity
        );

        pfm_memory_decommit(mem, ctx->mem.granularity);

        stack.ids[stack.position++] = block;
    }

    IFB_INTERNAL u32
    renderer_memory_element_count(
        renderer_context* ctx,
        const u32         element_size) {

        assert(ctx != NULL && element_size != 0);

        const u32 element_count = (ctx->mem.granularity / element_size);
        assert(element_count != 0);
        
        return(element_count);
    }
};