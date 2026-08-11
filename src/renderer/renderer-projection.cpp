#pragma once

#include "renderer.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math.hpp"
#include <cassert>
#include <cmath>
#include <math.h>

namespace ifb {

    struct renderer_projection {
        f32 viewport_width;
        f32 viewport_height;
        f32 fov_y;
        f32 clip_far;
        f32 clip_near;
    };
    
    IFB_INTERNAL void
    renderer_projection_init(
        void) {

        assert(_renderer_ctx); 
        assert(_renderer_ctx->proj == NULL); 

        auto proj = _renderer_ctx->memory.stack.push_struct<renderer_projection>();
        assert(proj);

        proj->fov_y     = trig_degrees_to_radians(60.0f);
        proj->clip_near = 0.1f;
        proj->clip_far  = 1000.0f;
    
        _renderer_ctx->proj = proj;
    }
    
    IFB_INTERNAL void
    renderer_projection_set_viewport(
        const u32 width,
        const u32 height) {
        
        assert(_renderer_ctx);

        const bool can_resize = (
            _renderer_ctx->gl   != NULL &&
            width               != 0    &&
            height              != 0    &&
            _renderer_ctx->proj != NULL         
        );
        if (!can_resize) return;

        gl_context_update_viewport(
            _renderer_ctx->gl,
            0,0,
            width,
            height
        );

        auto proj = _renderer_ctx->proj; 
        assert(proj);
        proj->viewport_width  = width;
        proj->viewport_height = height;
    }

    IFB_INTERNAL f32
    renderer_projection_get_aspect_ratio(
        void) {

        assert(_renderer_ctx);

        const auto proj         = _renderer_ctx->proj; 
        const f32  aspect_ratio = (proj->viewport_width / proj->viewport_height); 

        return(aspect_ratio);
    }

    IFB_INTERNAL mat4 
    renderer_projection_xform(
        void) {

        assert(_renderer_ctx);

        auto proj = _renderer_ctx->proj; 
        assert(proj);

        const f32 aspect          = renderer_projection_get_aspect_ratio(); 
        const f32 f               = 1.0f / tanf(proj->fov_y * 0.5f);
        const f32 f_div_aspect    = f / aspect; 
        const f32 near_sub_far    = proj->clip_near - proj->clip_far;
        const f32 near_add_far    = proj->clip_near + proj->clip_far;
        const f32 near_mul_far_x2 = proj->clip_near * proj->clip_far * 2;

        mat4 xform = mat4_identity();
        xform.r0c0 = f_div_aspect;   
        xform.r1c1 = f; 
        xform.r2c2 = near_add_far    / near_sub_far;
        xform.r2c3 = near_mul_far_x2 / near_sub_far;
        xform.r3c2 = -1.0f;
    
        return(xform);
    }
};
