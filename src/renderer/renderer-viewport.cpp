#pragma once

#include "renderer.hpp"
#include "renderer-internal.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math.hpp"
#include "eng-internal.hpp"
#include <cassert>
#include <cmath>
#include <math.h>

namespace ifb {

    struct renderer_viewport {
        f32 width;
        f32 height;
        f32 fov_y;
        f32 clip_far;
        f32 clip_near;
    };
   
    IFB_INTERNAL renderer_viewport*
    renderer_viewport_create(
        void) {

        auto* viewport = (renderer_viewport*)global_alloc(sizeof(renderer_viewport)); 
        assert(viewport);

        return(viewport);
    }

    IFB_INTERNAL void
    renderer_viewport_init(
        renderer_viewport* viewport) {

        assert(viewport); 

        viewport->fov_y     = trig_degrees_to_radians(60.0f);
        viewport->clip_near = 0.01f;
        viewport->clip_far  = 100.0f;
    }
    
    IFB_INTERNAL void
    renderer_viewport_set_dimensions(
        renderer_viewport* viewport,
        gl_context*        gl,
        const u32          width,
        const u32          height) {
        
        assert(viewport);

        const bool can_resize = (
            width  != 0    &&
            height != 0
        );
        if (!can_resize) return;

        gl_context_update_viewport(
            gl,
            0,0,
            width,
            height
        );

        viewport->width  = width;
        viewport->height = height;
    }

    IFB_INTERNAL f32
    renderer_viewport_get_aspect_ratio(
        renderer_viewport* viewport) {

        assert(viewport);
        assert(viewport->width  != 0);
        assert(viewport->height != 0);

        const f32 aspect_ratio = (viewport->width / viewport->height); 

        return(aspect_ratio);
    }

    IFB_INTERNAL void  
    renderer_projection_xform(
        renderer_viewport* viewport,
        mat4&              xform) {

        assert(viewport);

        const f32 aspect          = renderer_projection_get_aspect_ratio(); 
        const f32 f               = 1.0f / tanf(viewport->fov_y * 0.5f);
        const f32 f_div_aspect    = f / aspect; 
        const f32 near_sub_far    = viewport->clip_near - viewport->clip_far;
        const f32 near_add_far    = viewport->clip_near + viewport->clip_far;
        const f32 near_mul_far_x2 = viewport->clip_near * viewport->clip_far * 2;

        xform      = mat4_identity();
        xform.r0c0 = f_div_aspect;   
        xform.r1c1 = f; 
        xform.r2c2 = near_add_far    / near_sub_far;
        xform.r2c3 = near_mul_far_x2 / near_sub_far;
        xform.r3c2 = -1.0f;
    }
};
