#pragma once

#include "ifb-engine.hpp"
#include "ifb-random.hpp"
#include "random-internal.hpp"

namespace ifb {

    IFB_ENGINE_API u32
    eng_random_u32(
        void) {

        const u32 value = random_u32();
        return(value);
    }

    IFB_ENGINE_API f32
    eng_random_f32(
        void) {

        const f32 value = random_f32();
        return(value);
    }

    IFB_ENGINE_API f32
    eng_random_f32_range(
        const f32 min,
        const f32 max) {

        const f32 value = random_f32_range(min, max);
        return(value);
    }
};
