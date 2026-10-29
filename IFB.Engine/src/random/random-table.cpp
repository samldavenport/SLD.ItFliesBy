#pragma once

#include <cassert>
#include "ifb-platform.hpp"
#include "eng-random.hpp"

#include "eng-context.hpp"
#include "ifb-config.hpp"
#include "eng-platform.hpp"
#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    struct random_table {
        u32  seed;
        u32  count;
        u32  position;
        u32  frame_count;
        u32* array_values;
    } static * _random_table;

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // xorshift, the seed can never be zero
    inline u32
    random_table_next_seed(
        u32 seed) {

        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        return(seed);
    }

    IFB_INTERNAL random_table*
    random_table_create(
        void) {

        const auto& cfg = config_instance();
        assert(cfg.random_table_count != 0);

        _random_table = global_alloc<random_table>();
        assert(_random_table);

        _random_table->seed         = (u32)pfm_system_timestamp() | 1;
        _random_table->count        = cfg.random_table_count;
        _random_table->array_values = global_alloc<u32>(cfg.random_table_count);
        assert(_random_table->array_values);

        random_table_refresh();

        return(_random_table);
    }

    IFB_INTERNAL void
    random_table_refresh(
        void) {

        assert(_random_table);
        assert(_random_table->array_values);

        // generate a new table of values
        u32 seed = _random_table->seed;
        for (
            u32 index = 0;
                index < _random_table->count;
              ++index) {

            seed = random_table_next_seed(seed);
            _random_table->array_values[index] = seed;
        }

        _random_table->seed        = seed;
        _random_table->position    = 0;
        _random_table->frame_count = 0;
    }

    IFB_INTERNAL void
    random_table_update(
        void) {

        assert(_random_table);

        const auto& cfg = config_instance();

        // the table is refreshed every so many frames
        ++_random_table->frame_count;
        if (_random_table->frame_count >= cfg.random_refresh_frames) {
            random_table_refresh();
        }
    }

    IFB_INTERNAL u32
    random_u32(
        void) {

        assert(_random_table);
        assert(_random_table->array_values);

        // the table is also refreshed when it is used up
        if (_random_table->position >= _random_table->count) {
            random_table_refresh();
        }

        const u32 value = _random_table->array_values[_random_table->position];
        ++_random_table->position;
        return(value);
    }

    IFB_INTERNAL f32
    random_f32(
        void) {

        // the top 24 bits fit in a float exactly
        const f32 value = (f32)(random_u32() >> 8) * (1.0f / 16777216.0f);
        return(value);
    }

    IFB_INTERNAL f32
    random_f32_range(
        const f32 min,
        const f32 max) {

        const f32 value = min + ((max - min) * random_f32());
        return(value);
    }
};
