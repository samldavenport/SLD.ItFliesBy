#pragma once

#include "eng-renderer.hpp"
#include "eng-scene.hpp"

namespace ifb {

    IFB_ENGINE_API bool
    eng_renderer_is_grid_visible(
        void) {

        const bool is_visible = renderer_is_grid_visible();
        return(is_visible);
    }

    IFB_ENGINE_API void
    eng_renderer_set_grid_visible(
        const bool is_visible) {

        renderer_set_grid_visible(is_visible);
    }

    // sets the parallax of the bound scene
    // the names are image assets, they all have to be the same size
    IFB_ENGINE_API bool
    eng_parallax_set(
        const cchar* front,
        const cchar* mid,
        const cchar* back) {

        assert(front && mid && back);

        const bool did_set = eng_scene_set_parallax(scene_get_bound(), front, mid, back);
        return(did_set);
    }
};
