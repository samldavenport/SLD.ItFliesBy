#pragma once

#include "renderer.hpp"
#include "memory.cpp" 
#include "quad.cpp" 
#include "hello-quad.cpp" 

namespace ifb {

    IFB_INTERNAL u32
    renderer_context_memory_requirement(
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
            sizeof(renderer_context)   +
            sizeof(gl_context) + 
            block_stack_size
        );

        return(mem_req);
    }

    IFB_INTERNAL renderer_context*
    renderer_context_init_from_memory(
        memory& mem) {

        assert(
            mem.size == renderer_context_memory_requirement() &&
            mem.ptr  != NULL
        );

        const addr addr_rndr      = mem.address;
        const addr addr_gl        = addr_rndr + sizeof(renderer_context);
        const addr addr_block_ids = addr_rndr + sizeof(gl_context);

        auto rndr                 =   (renderer_context*)addr_rndr;
        rndr->gl                  =         (gl_context*)addr_gl;
        rndr->mem.block_stack.ids =                (u32*)addr_block_ids;

        return(rndr);
    }

    IFB_INTERNAL void
    renderer_context_startup(
        renderer_context* ctx,
        memory&           reserved_memory) {

        const auto& cfg = config_instance();

        assert(
            ctx                  != NULL &&
            reserved_memory.size != 0    &&
            reserved_memory.ptr  != NULL          
        );

        // initialize opengl and imgui
        pfm_graphics_init_opengl(ctx->gl);
        pfm_graphics_init_imgui();

        // initialize memory
        ctx->mem.address              = reserved_memory.address;
        ctx->mem.size                 = reserved_memory.size;
        ctx->mem.granularity          = cfg.renderer_mem_granularity;
        ctx->mem.block_stack.capacity = reserved_memory.size / cfg.renderer_mem_granularity;
        ctx->mem.block_stack.position = ctx->mem.block_stack.capacity;

        for (
            u32 index = 0;
                index < ctx->mem.block_stack.capacity;
              ++index
        ) {
            ctx->mem.block_stack.ids[index] = index;
        }
    }

    IFB_INTERNAL void
    renderer_context_shutdown(
        renderer_context* ctx) {

        //TODO
    }
};