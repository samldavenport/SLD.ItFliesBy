#pragma once

#include "renderer.hpp"
#include "memory.cpp" 
#include "quad.cpp" 
#include "hello-quad.cpp" 
#include "direction-gizmo.cpp"
#include "camera.cpp"
#include "eng-internal.hpp"
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
    renderer_context_create(
        void) {

    }

    IFB_INTERNAL renderer_context*
    renderer_context_init_from_memory(
        memory& mem) {

        assert(
            mem.size == renderer_context_memory_requirement() &&
            mem.ptr  != NULL
        );

        auto rndr = global_alloc
        const addr addr_rndr      = mem.address;
        const addr addr_gl        = addr_rndr + sizeof(renderer_context);
        const addr addr_block_ids = addr_rndr + sizeof(gl_context);

        _renderer_ctx                      =   (renderer_context*)addr_rndr;
        _renderer_ctx->gl                  =         (gl_context*)addr_gl;
        _renderer_ctx->mem.block_stack.ids =                (u32*)addr_block_ids;

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

        // NOTE(SAM): the renderer doesn't need to initialize the opengl context
        // we can pass the context to the function and use it that way
        // same for imgui, it can be initialized externally

        // initialize opengl and imgui
        pfm_graphics_init_opengl(_renderer_ctx->gl);
        pfm_graphics_init_imgui();

        // initialize memory
        _renderer_ctx->mem.address              = reserved_memory.address;
        _renderer_ctx->mem.size                 = reserved_memory.size;
        _renderer_ctx->mem.granularity          = cfg.renderer_mem_granularity;
        _renderer_ctx->mem.block_stack.capacity = reserved_memory.size / cfg.renderer_mem_granularity;
        _renderer_ctx->mem.block_stack.position = _renderer_ctx->mem.block_stack.capacity;

        for (
            u32 index = 0;
                index < _renderer_ctx->mem.block_stack.capacity;
              ++index
        ) {
            _renderer_ctx->mem.block_stack.ids[index] = index;
        }

        // intialize camera
        renderer_camera_init();
    }

    IFB_INTERNAL void
    renderer_context_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void
    renderer_context_update_viewport(
        const u32         width,
        const u32         height) {


        assert(_renderer_ctx != NULL);

        // if any of these are null or 0,
        // we can just assume initialization
        // has not happened yet
        const bool can_resize = (
            _renderer_ctx->gl != NULL &&
            width   != 0    &&
            height  != 0
        );
        if (!can_resize) return;

        gl_context_update_viewport(
            _renderer_ctx->gl,
            0,0,
            width,
            height
        );

        _renderer_ctx->dims.width  = width;
        _renderer_ctx->dims.height = height;
    }

    IFB_INTERNAL void
    renderer_context_update_projection_matrix(
        void) {

        assert(_renderer_ctx);

        const f32 aspect_ratio = renderer_context_aspect_ratio();
        if (aspect_ratio == 0) {

            _renderer_ctx->xform_proj = mat4_identity();
        }

        // typical for most engines
        static const f32 clip_near   = 0.1f; 
        static const f32 fov_radians = trig_degrees_to_radians(45.0f);

        _renderer_ctx->xform_proj = xform_project_near_to_infinite(
            fov_radians,
            aspect_ratio,
            clip_near
        );
    }

    IFB_INTERNAL void
    renderer_context_update_view_matrix(
        void) {

        assert(_renderer_ctx);

        _renderer_ctx->xform_view = xform_view_look_at(
            _renderer_ctx->cam.origin,
            _renderer_ctx->cam.target
        );
    }

    IFB_INTERNAL f32
    renderer_context_aspect_ratio(
        void) {

        assert(_renderer_ctx);

        const f32 aspect_ratio = (_renderer_ctx->dims.height != 0) 
            ? (_renderer_ctx->dims.width / _renderer_ctx->dims.height)
            : 0;

        return(aspect_ratio);
    }
};