#pragma once

#include "renderer.hpp"

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

    IFB_ENGINE_API void
    eng_camera_get_xform(
        mat4& xform) {

        renderer_get_camera_xform(xform);
    }
};
