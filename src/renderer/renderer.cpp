#pragma once

#include "renderer.hpp"
#include "renderer-hello-quad.cpp" 
#include "renderer-quad.cpp" 
#include "renderer-direction-gizmo.cpp"
#include "renderer-camera.cpp"
#include "eng-internal.hpp"
#include "quad.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------
    
    IFB_INLINE void renderer_init_quad_memory(void);

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

        _renderer_ctx                      = rndr;
        _renderer_ctx->gl                  = gl;

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

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INLINE void
    renderer_init_quad_memory(
        void) {

        const auto& cfg             = config_instance();
        const u32   vertices_size   = (cfg.quad_capacity  * sizeof(quad_vertices));
        const u32   elements_count  = (cfg.quad_capacity  * 6);
        const u32   elements_size   = (elements_count     * sizeof(u32));
        void*       vertices_memory = _renderer_ctx->memory.stack.push(vertices_size);
        void*       elements_memory = _renderer_ctx->memory.stack.push(elements_size);
        auto        list_elements   = _renderer_ctx->memory.stack.push_struct<entity_id>(cfg.quad_capacity);

        assert(vertices_size   != 0);
        assert(vertices_memory != NULL); 
        assert(list_elements   != NULL); 

        quad_buffers& buffers = _renderer_ctx->shader.quad.buffers;
        buffers.vertices.size = vertices_size; 
        buffers.vertices.vptr = vertices_memory;
        buffers.elements.size = elements_size;
        buffers.elements.vptr = elements_memory;

        // we can initialize the element buffer array
        // because they always repeat
        for (
            u32 i = 0;
                i < elements_count;
              ++i) {

            quad_elements& curr = buffers.elements.array[i];

            curr.triangle_1.elmnt_0_index_0 = 0;
            curr.triangle_1.elmnt_1_index_1 = 1;
            curr.triangle_1.elmnt_2_index_3 = 3;
            curr.triangle_2.elmnt_3_index_1 = 1;
            curr.triangle_2.elmnt_4_index_2 = 2;
            curr.triangle_2.elmnt_5_index_3 = 3;
        }

        // initialize the quad list
        _renderer_ctx->shader.quad.list.init(list_elements, cfg.quad_capacity);
    }
};