#pragma once

#include "ifb-types.hpp"
#include "renderer.hpp"
#include "renderer-internal.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math-vec3.hpp"
#include "eng-internal.hpp"
#include <cassert>

namespace ifb {

    struct renderer_camera {
        vec3 origin;
        vec3 target;
    };


    IFB_INTERNAL renderer_camera*
    renderer_camera_create(
        void) {
        
        auto cam = (renderer_camera*)global_alloc(sizeof(renderer_camera));
        return(cam);
    }
    
    IFB_INTERNAL void
    renderer_camera_init(
        renderer_camera* camera) {

        assert(camera);

        camera->origin = { 0.0f, 0.3f, 0.6f };
        camera->target = { 0.0f, 0.2f, 0.1f };
    }

    IFB_INTERNAL const vec3& 
    renderer_camera_get_origin(
        renderer_camera* camera) {

        assert(camera);
        return(camera->origin);
    }

    IFB_INTERNAL const vec3& 
    renderer_camera_get_target(
        renderer_camera* camera) {

        assert(camera); 
        return(camera->target); 
    }

    IFB_INTERNAL void
    renderer_camera_set_origin(
        renderer_camera* camera,
        const vec3&      origin) {

        assert(camera);
        camera->origin = origin;
    }

    IFB_INTERNAL void
    renderer_camera_set_target(
        renderer_camera* camera,
        const vec3&      target) {

        assert(camera);
        camera->target = target;
    }

    IFB_INTERNAL void
    renderer_camera_get_orientation(
        renderer_camera* camera,
        orientation&     ori) {

        assert(camera);
       
        const vec3 target_sub_origin = vec3_subtract(camera->target, camera->origin);  
        ori.forward  = vec3_normalize(target_sub_origin);         
   
        static const vec3 world_up = { 0.0f, 1.0f, 0.0f };
        const vec3 forward_cross_world_up = vec3_cross(ori.forward, world_up);
        ori.right = vec3_normalize(forward_cross_world_up);          
       
        const vec3 right_cross_forward = vec3_cross(ori.right, ori.forward);
        ori.up = vec3_normalize(right_cross_forward); 
    }
    
    IFB_INTERNAL void 
    renderer_camera_xform(
        renderer_camera* camera,
        mat4&            xform) {

        assert(camera);
       
        // get the orientation
        orientation ori;
        renderer_camera_get_orientation(camera, ori);

        xform      = mat4_identity();

        xform.r0c0 = ori.right.x;
        xform.r0c1 = ori.right.y;
        xform.r0c2 = ori.right.z;
        xform.r0c3 = -vec3_dot(ori.right, camera->origin);

        xform.r1c0 = ori.up.x;
        xform.r1c1 = ori.up.y;
        xform.r1c2 = ori.up.z;
        xform.r1c3 = -vec3_dot(ori.up, camera->origin);
        
        xform.r2c0 = -ori.forward.x;
        xform.r2c1 = -ori.forward.y;
        xform.r2c2 = -ori.forward.z;
        xform.r2c3 =  vec3_dot(ori.forward, camera->origin);
    
        xform.r3c0 = 0.0f; 
        xform.r3c1 = 0.0f; 
        xform.r3c2 = 0.0f; 
        xform.r3c3 = 1.0f;
    }
};
