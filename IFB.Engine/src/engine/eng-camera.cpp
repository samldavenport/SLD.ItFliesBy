#pragma once

#include "renderer-internal.hpp"

namespace ifb {

    IFB_ENGINE_API const vec3& 
    eng_camera_get_origin(
        void) {

        const vec3& origin = renderer_get_camera_origin();
        return(origin);
    }

    IFB_ENGINE_API const vec3& 
    eng_camera_get_target(
        void) {

        const vec3& target = renderer_get_camera_target();
        return(target);
    }

    IFB_ENGINE_API void
    eng_camera_set_origin(
        const vec3& origin) {

        renderer_set_camera_origin(origin);
    }

    IFB_ENGINE_API void
    eng_camera_set_target(
        const vec3& target) {

        renderer_set_camera_target(target);
    }

    // the vertical field of view, in degrees
    IFB_ENGINE_API f32
    eng_camera_get_fov(
        void) {

        const f32 fov_y = trig_radians_to_degrees(renderer_get_viewport_fov());
        return(fov_y);
    }

    IFB_ENGINE_API void
    eng_camera_set_fov(
        const f32 fov_y) {

        renderer_set_viewport_fov(trig_degrees_to_radians(fov_y));
    }

    IFB_ENGINE_API void
    eng_camera_get_xform(
        mat4& xform) {

        renderer_get_camera_xform(xform);
    }
};
