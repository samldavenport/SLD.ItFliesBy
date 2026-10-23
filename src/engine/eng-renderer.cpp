#pragma once

#include "renderer.hpp"

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
};
