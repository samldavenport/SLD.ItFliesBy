#pragma once

#include "eng-internal.hpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_camera_get_origin(
        vec3& origin) {

        renderer_camera_get_origin(origin);
    }

    IFB_ENGINE_API void
    eng_camera_get_target(
        vec3& target) {

        renderer_camera_get_target(target);
    }

    IFB_ENGINE_API void
    eng_camera_set_origin(
        const vec3& origin) {

        renderer_camera_set_origin(origin);
    }

    IFB_ENGINE_API void
    eng_camera_set_target(
        const vec3& target) {

        renderer_camera_set_target(target);
    }
};