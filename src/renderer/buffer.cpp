#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL quad_buffer*
    renderer_quad_buffer_create(
        renderer_context* ctx) {

        assert(ctx);

        const u32 mem_size = ctx->mem.granularity;
        void*     mem      = renderer_memory_commit(ctx);
        assert(mem);

        zero_memory(mem, mem_size);

        auto qb      = (quad_buffer*)mem;
        qb->ptr      = (quad*)((addr)mem + sizeof(quad_buffer));
        qb->capacity = (mem_size - sizeof(quad_buffer)) / sizeof(quad);  
        qb->count    = 0;
        qb->gl_buf   = gl_buffer_create(ctx->gl);

        assert(
            qb->ptr      != NULL &&
            qb->capacity != 0    &&
            qb->gl_buf   != GL_ID_INVALID
        );

        return(qb);
    }

    IFB_INTERNAL void
    renderer_quad_buffer_destroy(
        renderer_context* ctx,
        quad_buffer* qb) {

        assert(
            ctx != NULL &&
            qb   != NULL &&
            gl_buffer_destroy(ctx->gl, qb->gl_buf);
        );

        renderer_memory_decommit(ctx, (void*)qb);
    }
};