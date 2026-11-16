#pragma once

#include "ifb-engine.hpp"
#include "physics-manager.cpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_physics_get_stats(
        eng_physics_stats& stats) {

        stats = phys_mngr_get_stats();
    }
};
