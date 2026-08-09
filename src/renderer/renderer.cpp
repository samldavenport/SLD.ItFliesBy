#pragma once

#include "renderer.hpp"
#include "renderer-quad.cpp" 
#include "renderer-camera.cpp"
#include "renderer-projection.cpp"
#include "eng-internal.hpp"
#include "sld-math-mat4.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------
    
    IFB_INLINE void renderer_init_quad_memory      (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL renderer_context*
    renderer_context_create(
        void) {

        const auto& cfg         = config_instance();
        const u32   block_count = (cfg.memory_size_rendering / cfg.renderer_mem_granularity);

        auto rndr      = global_alloc<renderer_context> ();
        auto gl        = global_alloc<gl_context>       ();
        auto block_ids = global_alloc<u32>              (block_count); 
        assert(
            rndr      != NULL &&
            gl        != NULL &&
            block_ids != NULL            
        );

        _renderer_ctx     = rndr;
        _renderer_ctx->gl = gl;

        return(_renderer_ctx);
    }

    IFB_INTERNAL void
    renderer_context_startup(
        memory& reserved_memory) {

        const auto& cfg = config_instance();

        assert(
            _renderer_ctx        != NULL &&
            reserved_memory.size != 0    &&
            reserved_memory.ptr  != NULL          
        );

        // create the stack
        memory commit;
        commit.size = reserved_memory.size;
        commit.ptr  = pfm_memory_commit(reserved_memory.ptr, 0, commit.size);
        assert(commit.size    != 0); 
        assert(commit.address != 0); 
        _renderer_ctx->memory.stack.init(commit);

        // initialize buffers
        renderer_init_quad_memory();

        // NOTE(SAM): the renderer doesn't need to initialize the opengl context
        // we can pass the context to the function and use it that way
        // same for imgui, it can be initialized externally

        // initialize opengl and imgui
        pfm_graphics_init_opengl(_renderer_ctx->gl);
        pfm_graphics_init_imgui();


        // intialize camera
        renderer_projection_init();
        renderer_camera_init();
        renderer_projection_set_viewport(cfg.window_start_width, cfg.window_start_height);
    }

    IFB_INTERNAL void
    renderer_context_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void*
    renderer_context_memory_alloc(
        const u32 size) {

        assert(size != 0);

        void* mem = _renderer_ctx->memory.stack.push(size);
        return(mem);
    }

    IFB_INTERNAL void
    renderer_context_draw_buffers(
        void) {

        // calculate view and projection matrices
        static mat4 proj = mat4_identity();
        static mat4 view = mat4_identity();
        renderer_projection_xform (proj);
        renderer_camera_xform     (view);
    
        renderer_quad_draw(view, proj);
    }
    
    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    renderer_init_quad_memory(
        void) {

        const auto& cfg     = config_instance();
        auto&       buffers = _renderer_ctx->shader.quad.buffers;
        auto&       list    = _renderer_ctx->shader.quad.render_list;

        auto* quad_entities       = (entity_id*)renderer_context_memory_alloc(cfg.quad_capacity * sizeof(entity_id));
        buffers.vertex.size       = (cfg.quad_capacity  * sizeof(renderer_quad_vertices)); 
        buffers.vertex.data.vptr  = renderer_context_memory_alloc(buffers.vertex.size);
        buffers.element.size      = (cfg.quad_capacity * sizeof(u32) * 6);
        buffers.element.data.vptr = renderer_context_memory_alloc(buffers.element.size);

        assert(buffers.vertex.size       != 0);
        assert(buffers.vertex.data.vptr  != 0);
        assert(buffers.element.size      != 0);
        assert(buffers.element.data.vptr != 0);
        assert(quad_entities             != NULL);

        list.stack_init(_renderer_ctx->memory.stack);
    }
};
