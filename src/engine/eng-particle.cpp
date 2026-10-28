#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "ifb-particle.hpp"
#include "renderer.hpp"

namespace ifb {

    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_create_parabolic(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       spawn_radius,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_parabolic_props& props) {

        assert(name           != NULL);
        assert(count          >  0);
        assert(lifespan_ms    >  0);
        assert(spawn_delay_ms >= 0);
        assert(spawn_radius   >= 0);
        assert(width          >  0);
        assert(height         >  0);

        const hnd_particle_emitter h_emitter = particle_emitter_create_parabolic(
            name,
            count,
            lifespan_ms,
            spawn_delay_ms,
            spawn_radius,
            width,
            height,
            color_primary,
            color_secondary,
            props
        );

        return(h_emitter);
    }

    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_create_radial(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       spawn_radius,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_radial_props&    props) {

        assert(name           != NULL);
        assert(count          >  0);
        assert(lifespan_ms    >  0);
        assert(spawn_delay_ms >= 0);
        assert(spawn_radius   >= 0);
        assert(width          >  0);
        assert(height         >  0);

        const hnd_particle_emitter h_emitter = particle_emitter_create_radial(
            name,
            count,
            lifespan_ms,
            spawn_delay_ms,
            spawn_radius,
            width,
            height,
            color_primary,
            color_secondary,
            props
        );

        return(h_emitter);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_destroy(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_destroy = particle_emitter_destroy(h_emitter);
        return(did_destroy);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_start(
        const hnd_particle_emitter h_emitter,
        const vec3&                position) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_start = particle_emitter_start(h_emitter, position);
        return(did_start);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_stop(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_stop = particle_emitter_stop(h_emitter);
        return(did_stop);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_render(
        const hnd_particle_emitter h_emitter) {

        //TODO(SLD): implement
        return(false);
    }

    IFB_ENGINE_API u32
    eng_particle_emitter_get_capacity(
        void) {

        const u32 capacity = particle_mngr_get_emitter_capacity();
        return(capacity);
    }

    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_get_hnd(
        const u32 index) {

        const hnd_particle_emitter h_emitter = particle_mngr_get_emitter_hnd(index);
        return(h_emitter);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_get_info(
        const hnd_particle_emitter h_emitter,
        particle_emitter_info&     info) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_get = particle_emitter_get_info(h_emitter, info);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_update(
        const hnd_particle_emitter   h_emitter,
        const particle_emitter_info& info) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_update = particle_emitter_update(h_emitter, info);
        return(did_update);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_get_props_parabolic(
        const hnd_particle_emitter h_emitter,
        particle_parabolic_props&  props) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_get = particle_emitter_get_props_parabolic(h_emitter, props);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_set_props_parabolic(
        const hnd_particle_emitter      h_emitter,
        const particle_parabolic_props& props) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_set = particle_emitter_set_props_parabolic(h_emitter, props);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_get_props_radial(
        const hnd_particle_emitter h_emitter,
        particle_radial_props&     props) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_get = particle_emitter_get_props_radial(h_emitter, props);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_set_props_radial(
        const hnd_particle_emitter   h_emitter,
        const particle_radial_props& props) {

        assert(h_emitter != INVALID_HANDLE);

        const bool did_set = particle_emitter_set_props_radial(h_emitter, props);
        return(did_set);
    }

    IFB_ENGINE_API void
    eng_particle_view_request(
        const u32 width,
        const u32 height) {

        renderer_request_particle_view(width, height);
    }

    IFB_ENGINE_API u32
    eng_particle_view_get_texture(
        void) {

        const u32 texture = renderer_get_particle_view_texture();
        return(texture);
    }

    IFB_ENGINE_API const vec3&
    eng_particle_view_get_camera_origin(
        void) {

        const vec3& origin = renderer_get_particle_view_camera_origin();
        return(origin);
    }

    IFB_ENGINE_API const vec3&
    eng_particle_view_get_camera_target(
        void) {

        const vec3& target = renderer_get_particle_view_camera_target();
        return(target);
    }

    IFB_ENGINE_API void
    eng_particle_view_set_camera_origin(
        const vec3& origin) {

        renderer_set_particle_view_camera_origin(origin);
    }

    IFB_ENGINE_API void
    eng_particle_view_set_camera_target(
        const vec3& target) {

        renderer_set_particle_view_camera_target(target);
    }
};
