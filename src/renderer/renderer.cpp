#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL u32
    renderer_memory_requirement(
        void) {

        const auto& cfg      = config_instance();
        const auto& mem_size = cfg.memory_size_rendering;
        const auto& mem_gran = cfg.renderer_mem_granularity;

        assert(
            mem_size            != 0 &&
            mem_gran            != 0 &&
            mem_size % mem_gran == 0
        );

        const u32 block_count      = (mem_size / mem_gran); 
        const u32 block_stack_size = (sizeof(u32) * block_count);
        const u32 mem_req          = (
            sizeof(renderer)   +
            sizeof(gl_context) + 
            block_stack_size
        );

        return(mem_req);
    }

    IFB_INTERNAL renderer*
    renderer_init_from_memory(
        memory& mem) {

        assert(
            mem.size == renderer_memory_requirement() &&
            mem.ptr  != NULL
        );

        const addr addr_rndr      = mem.address;
        const addr addr_gl        = addr_rndr + sizeof(renderer);
        const addr addr_block_ids = addr_rndr + sizeof(gl_context);

        auto rndr                 =   (renderer*)addr_rndr;
        rndr->gl                  = (gl_context*)addr_gl;
        rndr->mem.block_stack.ids =        (u32*)addr_block_ids;

        return(rndr);
    }

    IFB_INTERNAL void
    renderer_startup(
        renderer*  rndr,
        memory&    reserved_memory) {

        const auto& cfg = config_instance();

        assert(
            rndr                 != NULL &&
            reserved_memory.size != 0    &&
            reserved_memory.ptr  != NULL          
        );

        // initialize opengl
        pfm_graphics_init_opengl(rndr->gl);

        // initialize memory
        rndr->mem.address              = reserved_memory.address;
        rndr->mem.size                 = reserved_memory.size;
        rndr->mem.granularity          = cfg.renderer_mem_granularity;
        rndr->mem.block_stack.capacity = reserved_memory.size / cfg.renderer_mem_granularity;
        rndr->mem.block_stack.position = rndr->mem.block_stack.capacity;

        for (
            u32 index = 0;
                index < rndr->mem.block_stack.capacity;
              ++index
        ) {
            rndr->mem.block_stack.ids[index] = index;
        }
    }

    IFB_INTERNAL void*
    renderer_memory_commit(
        renderer* rndr) {

        assert(rndr);

        auto& mem = rndr->mem;
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
        renderer* rndr, void* mem) {

        assert(rndr != NULL && mem != NULL);

        auto& rnd_mem = rndr->mem;
        assert(
            rnd_mem.ptr                        != NULL                                 &&
            rnd_mem.size                       != 0                                    &&
            rnd_mem.granularity                != 0                                    &&
            rnd_mem.block_stack.ids            != 0                                    &&
            rnd_mem.block_stack.capacity       == (rnd_mem.size / rnd_mem.granularity) &&
            rnd_mem.block_stack.position       <  rnd_mem.block_stack.capacity         &&
            rnd_mem.size % rnd_mem.granularity == 0
        );

        auto& stack = rndr->mem.block_stack;

        const addr mem_addr = (addr)mem;
        const addr offset   = (mem_addr - rnd_mem.address);
        const u32  block    = (offset   / rnd_mem.granularity);
        assert(
            rnd_mem.granularity % offset == 0 &&
            block < stack.capacity
        );

        pfm_memory_decommit(mem);

        stack.ids[stack.position++] = block;
    }

    IFB_INTERNAL void
    renderer_shutdown(
        renderer* rndr) {

        //TODO
    }
};