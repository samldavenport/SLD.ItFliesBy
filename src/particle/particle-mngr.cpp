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
        f32*                  array_life_ms;
        color_rgba_u32*       array_colors;
        vec3*                 array_position;
    } static * _particle_mngr;

    // the unaligned size of every array in the manager
    struct particle_mngr_sizes {
        u32 hnd;
        u32 emitters;
        u32 life_ms;
        u32 colors;
        u32 position;
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
        sizes.hnd      = capacity       * sizeof(hnd_particle_emitter);
        sizes.emitters = capacity       * sizeof(particle_emitter);
        sizes.life_ms  = particle_count * sizeof(f32);
        sizes.colors   = particle_count * sizeof(color_rgba_u32);
        sizes.position = particle_count * sizeof(vec3);
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
            sizes.emitters,
            sizes.life_ms,
            sizes.colors,
            sizes.position
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
        _particle_mngr->array_life_ms  =                  (f32*)reservation_push_bytes(res, sizes.life_ms);
        _particle_mngr->array_colors   =       (color_rgba_u32*)reservation_push_bytes(res, sizes.colors);
        _particle_mngr->array_position =                 (vec3*)reservation_push_bytes(res, sizes.position);
        assert(_particle_mngr->array_hnd);
        assert(_particle_mngr->array_emitters);
        assert(_particle_mngr->array_life_ms);
        assert(_particle_mngr->array_colors);
        assert(_particle_mngr->array_position);

        // point each emitter at its slice of the particle arrays
        for (
            u32 emitter_index = 0;
                emitter_index < _particle_mngr->capacity;
              ++emitter_index) {

            const u32         particle_index = emitter_index * cfg.particle_bufffer_count;
            particle_emitter& emitter        = _particle_mngr->array_emitters[emitter_index];

            _particle_mngr->array_hnd[emitter_index] = INVALID_HANDLE;
            emitter.hnd            = INVALID_HANDLE;
            emitter.array_life_ms  = &_particle_mngr->array_life_ms  [particle_index];
            emitter.array_colors   = &_particle_mngr->array_colors   [particle_index];
            emitter.array_position = &_particle_mngr->array_position [particle_index];
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
    
    IFB_INTERNAL particle_emitter&
    particle_mngr_set_handle(
        const u32                  index,
        const hnd_particle_emitter hnd) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_hnd);
        assert(_particle_mngr->array_emitters);
        assert(_particle_mngr->capacity > index);

        // the handle array and the emitter always agree
        auto& emitter = _particle_mngr->array_emitters[index];
        _particle_mngr->array_hnd[index] = hnd;
        emitter.hnd                      = hnd;
        return(emitter);
    }
    
    IFB_INTERNAL u32
    particle_mngr_find_emitter_index_new(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        u32 emitter_index_new = INVALID_INDEX; 
        for (
            u32 emitter_index_curr = 0;
                emitter_index_curr < _particle_mngr->capacity;
              ++emitter_index_curr) {

            const hnd_particle_emitter hnd_curr = _particle_mngr->array_hnd[emitter_index_curr]; 
            assert(h_emitter != hnd_curr);
            if (emitter_index_new == INVALID_INDEX && hnd_curr == INVALID_HANDLE) {
                emitter_index_new = emitter_index_curr;
            }
        }   

        return(emitter_index_new);
    }
    
    IFB_INTERNAL u32
    particle_mngr_find_emitter_index_existing(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        u32 emitter_index = INVALID_INDEX; 
        for (
            u32 emitter_index_curr = 0;
                emitter_index_curr < _particle_mngr->capacity;
              ++emitter_index_curr) {

            if (h_emitter == _particle_mngr->array_hnd[emitter_index_curr]) {
                emitter_index = emitter_index_curr;
                break;
            }
        }

        return(emitter_index);
    }
    
    IFB_INTERNAL particle_emitter& 
    particle_mngr_get_emitter(
        const u32 index) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_emitters);
        assert(_particle_mngr->capacity > index);

        auto& emitter = _particle_mngr->array_emitters[index];
        return(emitter);
    }

};
