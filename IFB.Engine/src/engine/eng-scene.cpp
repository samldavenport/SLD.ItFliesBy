#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-entity.hpp"
#include "eng-scene.hpp"
#include "eng-asset.hpp"
#include "asset-database.cpp"
#include "scene-mngr.cpp"

namespace ifb {

    IFB_ENGINE_API hnd_scene
    eng_scene_create(
        const cchar* name) {

        assert(name);
        return(scene_create(name));
    }

    IFB_ENGINE_API bool
    eng_scene_destroy(
        const hnd_scene h_scene) {

        return(scene_destroy(h_scene));
    }

    IFB_ENGINE_API bool
    eng_scene_reset(
        const hnd_scene h_scene) {

        return(scene_reset(h_scene));
    }

    IFB_ENGINE_API bool
    eng_scene_bind(
        const hnd_scene h_scene) {

        return(scene_bind(h_scene));
    }

    IFB_ENGINE_API hnd_scene
    eng_scene_get_bound(
        void) {

        return(scene_get_bound());
    }

    IFB_ENGINE_API hnd_scene
    eng_scene_get_default(
        void) {

        return(scene_get_default());
    }

    IFB_ENGINE_API const cchar*
    eng_scene_get_name(
        const hnd_scene h_scene) {

        return(scene_get_name(h_scene));
    }

    IFB_ENGINE_API u32
    eng_scene_get_entity_count(
        const hnd_scene h_scene) {

        const u32 count = scene_is_valid(h_scene)
            ? entity_count_by_scene(h_scene)
            : 0;
        return(count);
    }

    IFB_ENGINE_API void*
    eng_scene_alloc(
        const hnd_scene h_scene,
        const u32       size) {

        return(scene_alloc(h_scene, size));
    }

    IFB_ENGINE_API bool
    eng_scene_push(
        const hnd_scene       h_scene,
        const scene_flags_u32 flags) {

        return(scene_push(h_scene, flags));
    }

    IFB_ENGINE_API bool
    eng_scene_pop(
        void) {

        return(scene_pop());
    }

    IFB_ENGINE_API bool
    eng_scene_transition(
        const hnd_scene       h_scene,
        const scene_flags_u32 flags) {

        return(scene_transition(h_scene, flags));
    }

    // the names are image assets, they all have to be the same size
    // they're loaded when the scene is the highest rendering scene with a parallax
    IFB_ENGINE_API bool
    eng_scene_set_parallax(
        const hnd_scene h_scene,
        const cchar*    front,
        const cchar*    mid,
        const cchar*    back) {

        assert(front && mid && back);

        // catch a bad name now, the layers load later
        const asset_database* db = asset_mngr_get_database();
        if (db == NULL) {
            return(false);
        }
        // the same lookup the renderer uses to load them
        asset_index index;
        const bool do_assets_exist = (
            asset_database_find(db, asset_database_handle_of(front), asset_type_e_image, index) &&
            asset_database_find(db, asset_database_handle_of(mid),   asset_type_e_image, index) &&
            asset_database_find(db, asset_database_handle_of(back),  asset_type_e_image, index)
        );
        if (!do_assets_exist) {
            return(false);
        }

        // the scene stores the layers back to front
        return(scene_set_parallax(h_scene, back, mid, front));
    }

    IFB_ENGINE_API bool
    eng_scene_clear_parallax(
        const hnd_scene h_scene) {

        return(scene_clear_parallax(h_scene));
    }

    IFB_ENGINE_API hnd_scene
    eng_scene_get_top(
        void) {

        return(scene_get_top());
    }

    IFB_ENGINE_API u32
    eng_scene_get_stack_count(
        void) {

        return(scene_get_stack_count());
    }

    IFB_ENGINE_API bool
    eng_scene_is_updating(
        const hnd_scene h_scene) {

        return(scene_is_updating(h_scene));
    }

    IFB_ENGINE_API bool
    eng_scene_is_rendering(
        const hnd_scene h_scene) {

        return(scene_is_rendering(h_scene));
    }

    IFB_ENGINE_API hnd_scene
    eng_entity_get_scene(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);
        return(entity_lookup_scene(id));
    }
};
