#pragma once

#include "renderer.hpp"
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

        cam->origin = { 0.5f, 0.5f, -0.5f };
        cam->target = { 0.0f, 0.0f,  0.0f };
    
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
    
    }

    IFB_INTERNAL void
    renderer_camera_get_right(
        vec3& right) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);
    }

    IFB_INTERNAL void
    renderer_camera_get_up(
        vec3& up) {

        assert(_renderer_ctx != NULL);
        auto cam = _renderer_ctx->cam;
        assert(cam);
    }

    IFB_INTERNAL void
    renderer_camera_xform(
        mat4& xform) {

        assert(_renderer_ctx);

        auto cam = _renderer_ctx->cam;
        assert(cam);

        xform = xform_view_look_at(
            cam->origin,
            cam->target
        );
    }
};
