#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL void
    renderer_camera_init(
        renderer_context* ctx) {

        assert(ctx);

        auto& cam = ctx->cam;

        cam.origin = { 0.5f, 0.5f, -0.5f };
        cam.target = { 0.0f, 0.0f,  0.0f };
    }


    IFB_INTERNAL void
    renderer_camera_get_origin(
        renderer_context* ctx,
        vec3&             origin) {

        assert(ctx != NULL);

        origin = ctx->cam.origin;
    }

    IFB_INTERNAL void
    renderer_camera_get_target(
        renderer_context* ctx,
        vec3&             target) {

        assert(ctx != NULL);

        target = ctx->cam.target;
    }

    IFB_INTERNAL void
    renderer_camera_set_origin(
        renderer_context* ctx,
        const vec3&       origin) {

        assert(ctx != NULL);

        ctx->cam.origin = origin;
    }

    IFB_INTERNAL void
    renderer_camera_set_target(
        renderer_context* ctx,
        const vec3&       target) {

        assert(ctx != NULL);

        ctx->cam.target = target;
    }

    IFB_INTERNAL void
    renderer_camera_get_view(
        renderer_context* ctx,
        view&             v) {

        assert(ctx != NULL);

        auto& cam = ctx->cam;

        v.look_at(cam.origin, cam.target);
    }

    IFB_INTERNAL void
    renderer_camera_get_forward(
        renderer_context* ctx,
        vec3&             forward) {

        assert(ctx != NULL);

    }

    IFB_INTERNAL void
    renderer_camera_get_right(
        renderer_context* ctx,
        vec3&             right) {

        assert(ctx != NULL);

    }

    IFB_INTERNAL void
    renderer_camera_get_up(
        renderer_context* ctx,
        vec3&             up) {

        assert(ctx != NULL);

    }

};