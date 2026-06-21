#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL void
    renderer_camera_set_origin(
        renderer_context* ctx,
        const vec3*       origin) {

        assert(ctx != NULL && origin != NULL);

        ctx->cam.origin = *origin;
    }

    IFB_INTERNAL void
    renderer_camera_set_target(
        renderer_context* ctx,
        const vec3*       target) {

        assert(ctx != NULL && target != NULL);

        ctx->cam.target = *target;
    }

    IFB_INTERNAL void
    renderer_camera_xform_look_at(
        renderer_context* ctx,
        mat4*             xform) {

        assert(ctx != NULL && xform != NULL);

        xform_look_at(
            xform,
            &ctx->cam.origin,
            &ctx->cam.target
        );
    }

    IFB_INTERNAL void
    renderer_camera_xform_view(
        renderer_context* ctx,
        mat4*             xform) {

        assert(ctx != NULL && xform != NULL);

        xform_look_at(
            xform,
            &ctx->cam.origin,
            &ctx->cam.target
        );

        mat4_transpose(xform, xform);
    }
};