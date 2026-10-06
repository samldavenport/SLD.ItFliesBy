#pragma once

#include <cassert>

#include "eng-internal.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "sld-math-types.hpp"
#include "sld-strings.hpp"
#include "sld.hpp"

namespace ifb {

    struct particle_mngr {
        reservation*          res;
        u32                   capacity;
        hnd_particle_emitter* array_hnd;
        particle_emitter*     array_emitters;
    } static * _particle_mngr;

    // the unaligned size of every array in the manager
    struct particle_mngr_sizes {
        u32 hnd;
        u32 emitters;
    };

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // this is the only place that knows how big the arrays are,
    // a new array needs to be added here and to the total below
    inline particle_mngr_sizes
    particle_mngr_calculate_sizes(
        const u32 capacity) {

        // every emitter gets a full buffer of particles
        const auto& cfg            = config_instance();
        const u32   particle_count = capacity * cfg.particle_bufffer_count;

        particle_mngr_sizes sizes;
        sizes.hnd      = capacity  * sizeof(hnd_particle_emitter);
        sizes.emitters = capacity  * sizeof(particle_emitter);
        return(sizes);
    }

    // the total size of the arrays as the reservation will commit them
    // each array is pushed on its own, so each one is aligned to a page on its own
    inline u32
    particle_mngr_calculate_size_total(
        const particle_mngr_sizes& sizes,
        const bool                 is_aligned) {

        const u32 size_array[] = {
            sizes.hnd,
            sizes.emitters
        };

        u32 size_total = 0;
        for (const u32 size : size_array) {
            size_total += is_aligned ? system_align_to_memory_page_size(size) : size;
        }
        return(size_total);
    }

    // the most emitters we can fit in what is left of the reservation
    inline u32
    particle_mngr_calculate_capacity(
        const reservation* res) {

        const u32 size_res     = reservation_get_capacity(res) - reservation_get_size_used(res);
        const u32 size_emitter = particle_mngr_calculate_size_total(particle_mngr_calculate_sizes(1), false);
        assert(size_emitter != 0);

        // aligning can only make the arrays bigger, so the unaligned capacity
        // is the upper bound, and we step down until the aligned arrays fit
        u32 capacity = size_res / size_emitter;
        while (
            capacity > 0 &&
            particle_mngr_calculate_size_total(particle_mngr_calculate_sizes(capacity), true) > size_res) {

            --capacity;
        }
        return(capacity);
    }

    IFB_INTERNAL particle_mngr*
    particle_mngr_create(
        void) {

        _particle_mngr = global_alloc<particle_mngr>();
        assert(_particle_mngr);

        return(_particle_mngr);
    }

    IFB_INTERNAL void
    particle_mngr_startup(
        reservation* res) {

        assert(_particle_mngr != NULL);
        assert(res            != NULL);

        const auto& cfg = config_instance();

        _particle_mngr->res      = res;
        _particle_mngr->capacity = particle_mngr_calculate_capacity(res);
        assert(_particle_mngr->capacity != 0);

        const particle_mngr_sizes sizes = particle_mngr_calculate_sizes(_particle_mngr->capacity);

        // allocate memory
        _particle_mngr->array_hnd      = (hnd_particle_emitter*)reservation_push_bytes(res, sizes.hnd);
        _particle_mngr->array_emitters =     (particle_emitter*)reservation_push_bytes(res, sizes.emitters);
        assert(_particle_mngr->array_hnd);
        assert(_particle_mngr->array_emitters);

        // point each emitter at its slice of the particle arrays
        for (
            u32 emitter_index = 0;
                emitter_index < _particle_mngr->capacity;
              ++emitter_index) {

            const u32         particle_index = emitter_index * cfg.particle_bufffer_count;
            particle_emitter& emitter        = _particle_mngr->array_emitters[emitter_index];

            _particle_mngr->array_hnd[emitter_index] = INVALID_HANDLE;
            emitter.hnd            = INVALID_HANDLE;
        }
    }
    
    IFB_INTERNAL u32
    particle_mngr_get_emitter_capacity(
        void) {

        assert(_particle_mngr);
        return(_particle_mngr->capacity);
    }
    
    IFB_INTERNAL hnd_particle_emitter
    particle_mngr_get_emitter_hnd(
        const u32 index) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_hnd);
        assert(_particle_mngr->capacity > index);
    
        const hnd_particle_emitter hnd = _particle_mngr->array_hnd[index];  
        return(hnd);
    }

    IFB_INTERNAL void
    particle_mngr_set_emitter(
        const u32                  index,
        const hnd_particle_emitter hnd,
        const f32                  lifespan_ms,
        const f32                  height,
        const f32                  width,
        const color_rgba_u32       color_primary,
        const color_rgba_u32       color_secondary,
        const cstr_c16             name) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_emitters);
        assert(_particle_mngr->capacity > index); 
        assert(hnd         != INVALID_HANDLE);
        assert(lifespan_ms != 0);
        assert(height      != 0);
        assert(width       != 0);

        _particle_mngr->array_hnd[index] = hnd;
        particle_emitter& emitter = _particle_mngr->array_emitters[index]; 
        emitter.hnd             = hnd;
        emitter.lifespan_ms     = lifespan_ms;
        emitter.height          = height;
        emitter.width           = width;
        emitter.color_primary   = color_primary;
        emitter.color_secondary = color_secondary;
        emitter.name            = name; 
   
        const auto& cfg = config_instance();
        zero_memory((void*)emitter.array_life_ms,  cfg.particle_bufffer_count * sizeof(f32)); 
        zero_memory((void*)emitter.array_colors,   cfg.particle_bufffer_count * sizeof(color_rgba_u32)); 
        zero_memory((void*)emitter.array_position, cfg.particle_bufffer_count * sizeof(vec3)); 
    }
};
