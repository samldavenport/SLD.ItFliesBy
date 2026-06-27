#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL void
    renderer_camera_init(
        void) {

        assert(_renderer_ctx);

        auto& cam = _renderer_ctx->cam;

        cam.origin = { 0.5f, 0.5f, -0.5f };
        cam.target = { 0.0f, 0.0f,  0.0f };
    }

    IFB_INTERNAL void
    renderer_camera_get_origin(
        vec3& origin) {

        assert(_renderer_ctx != NULL);

        origin = _renderer_ctx->cam.origin;
    }

    IFB_INTERNAL void
    renderer_camera_get_target(
        vec3& target) {

        assert(_renderer_ctx != NULL);

        target = _renderer_ctx->cam.target;
    }

    IFB_INTERNAL void
    renderer_camera_set_origin(
        const vec3& origin) {

        assert(_renderer_ctx != NULL);

        _renderer_ctx->cam.origin = origin;
    }

    IFB_INTERNAL void
    renderer_camera_set_target(
        const vec3& target) {

        assert(_renderer_ctx != NULL);

        _renderer_ctx->cam.target = target;
    }

    IFB_INTERNAL void
    renderer_camera_get_view(
        mat4& v) {

        assert(_renderer_ctx != NULL);

        auto& cam = _renderer_ctx->cam;

        v = xform_view_look_at(
            cam.origin,
            cam.target
        );
    }

    IFB_INTERNAL void
    renderer_camera_get_forward(
        vec3& forward) {

        assert(_renderer_ctx != NULL);
    }

    IFB_INTERNAL void
    renderer_camera_get_right(
        vec3& right) {

        assert(_renderer_ctx != NULL);
    }

    IFB_INTERNAL void
    renderer_camera_get_up(
        vec3& up) {

        assert(_renderer_ctx != NULL);
    }

};