#pragma once

#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math-vec3.hpp"
#include "sld-math-xforms.hpp"

namespace ifb {

    struct renderer_camera {
        vec3 origin;
        vec3 target;
    };

    IFB_INTERNAL void
    renderer_camera_init(
        void) {

        assert(_renderer_ctx);

        auto cam = _renderer_ctx->memory.stack.push_struct<renderer_camera>();
        assert(cam);

        cam->origin = { 0.0f, 0.0f, 0.5f };
        cam->target = { 0.0f, 0.0f, 0.0f };

        _renderer_ctx->cam = cam;
    }

    IFB_INTERNAL void
    renderer_camera_get_origin(
        vec3& origin) {

        assert(_renderer_ctx != NULL);
        
        auto cam = _renderer_ctx->cam;
        assert(cam);

        origin = cam->origin;
    }

    IFB_INTERNAL void
    renderer_camera_get_target(
        vec3& target) {

        assert(_renderer_ctx != NULL);

        auto cam = _renderer_ctx->cam;
        assert(cam);
        
        target = cam->target;
    }

    IFB_INTERNAL void
    renderer_camera_set_origin(
        const vec3& origin) {

        assert(_renderer_ctx != NULL);

        auto cam = _renderer_ctx->cam;
        assert(cam);

        cam->origin = origin;
    }

    IFB_INTERNAL void
    renderer_camera_set_target(
        const vec3& target) {

        assert(_renderer_ctx != NULL);

        auto cam = _renderer_ctx->cam;
        assert(cam);

        cam->target = target;
    }

    IFB_INTERNAL void
    renderer_camera_get_forward(
        vec3& forward) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);

        const vec3 target_sub_origin = vec3_subtract(cam->target, cam->origin);  
        forward = vec3_normalize(target_sub_origin);     
    }

    IFB_INTERNAL void
    renderer_camera_get_right(
        vec3& right) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);
   
        static const vec3 world_up = { 0.0f, 1.0f, 0.0f };


        vec3 forward;

        const vec3 forward_cross_world_up = vec3_cross(forward, world_up);
    }

    IFB_INTERNAL void
    renderer_camera_get_up(
        vec3& up) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);
    }

    IFB_INTERNAL void
    renderer_camera_get_orientation(
        orientation& o) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);
       
        const vec3 target_sub_origin = vec3_subtract(cam->target, cam->origin);  
        o.forward  = vec3_normalize(target_sub_origin);         
   
        static const vec3 world_up = { 0.0f, 1.0f, 0.0f };
        const vec3 forward_cross_world_up = vec3_cross(o.forward, world_up);
        o.right = vec3_normalize(forward_cross_world_up);          
       
        const vec3 right_cross_forward = vec3_cross(o.right, o.forward);
        o.up = vec3_normalize(right_cross_forward); 
    }
    
    IFB_INTERNAL mat4 
    renderer_camera_xform(
        void) {

        assert(_renderer_ctx);

        auto& cam = _renderer_ctx->cam;
        assert(cam);
       
        // get the orientation
        orientation ori;
        renderer_camera_get_orientation(ori);

        mat4 xform = mat4_identity();

        xform.r0c0 = ori.right.x;
        xform.r0c1 = ori.right.y;
        xform.r0c2 = ori.right.z;
        xform.r0c3 = -vec3_dot(ori.right, cam->origin);

        xform.r1c0 = ori.up.x;
        xform.r1c1 = ori.up.y;
        xform.r1c2 = ori.up.z;
        xform.r1c3 = -vec3_dot(ori.up, cam->origin);
        
        xform.r2c0 = -ori.forward.x;
        xform.r2c1 = -ori.forward.y;
        xform.r2c2 = -ori.forward.z;
        xform.r2c3 =  vec3_dot(ori.forward, cam->origin);
    
        xform.r3c0 = 0.0f; 
        xform.r3c1 = 0.0f; 
        xform.r3c2 = 0.0f; 
        xform.r3c3 = 1.0f;
    
        return(xform);
    }
};
