#pragma once

#include "eng-renderer.hpp"

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

    // the projection times the view, what the window is drawn with
    // that's the free camera while it's on
    IFB_ENGINE_API void
    eng_camera_get_view_proj(
        mat4& xform) {

        renderer_view_projection_xform(xform);
    }

    IFB_ENGINE_API void
    eng_camera_free_set_enabled(
        const bool is_enabled) {

        renderer_free_camera_set_enabled(is_enabled);
    }

    IFB_ENGINE_API bool
    eng_camera_free_is_enabled(
        void) {

        return(renderer_free_camera_is_enabled());
    }

    IFB_ENGINE_API void
    eng_camera_free_reset(
        void) {

        renderer_free_camera_reset();
    }

    IFB_ENGINE_API const vec3&
    eng_camera_free_get_origin(
        void) {

        return(renderer_free_camera_get_origin());
    }

    IFB_ENGINE_API const vec3&
    eng_camera_free_get_target(
        void) {

        return(renderer_free_camera_get_target());
    }

    IFB_ENGINE_API void
    eng_camera_free_set_origin(
        const vec3& origin) {

        renderer_free_camera_set_origin(origin);
    }

    IFB_ENGINE_API void
    eng_camera_free_set_target(
        const vec3& target) {

        renderer_free_camera_set_target(target);
    }

    IFB_ENGINE_API void
    eng_camera_free_get_xform(
        mat4& xform) {

        renderer_free_camera_get_xform(xform);
    }

    // the vertical field of view, in degrees
    IFB_ENGINE_API f32
    eng_camera_free_get_fov(
        void) {

        return(trig_radians_to_degrees(renderer_free_camera_get_fov()));
    }

    IFB_ENGINE_API void
    eng_camera_free_set_fov(
        const f32 fov_y) {

        renderer_free_camera_set_fov(trig_degrees_to_radians(fov_y));
    }
};
