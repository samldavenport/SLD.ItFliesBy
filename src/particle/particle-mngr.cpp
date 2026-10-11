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
        vec3*                 array_velocity;

        // type properties, one slot per emitter
        particle_parabolic_props* array_props_parabolic;
        particle_radial_props*    array_props_radial;

        // the active emitters of each type
        particle_emitter_list     array_lists[particle_emitter_type_e_count];
    } static * _particle_mngr;

    // the unaligned size of every array in the manager
    struct particle_mngr_sizes {
        u32 hnd;
        u32 emitters;
        u32 life_ms;
        u32 colors;
        u32 position;
        u32 velocity;
        u32 props_parabolic;
        u32 props_radial;
        u32 list;
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
        sizes.velocity = particle_count * sizeof(vec3);

        // every emitter has a slot for the props of each type,
        // and each list can hold every emitter
        sizes.props_parabolic = capacity * sizeof(particle_parabolic_props);
        sizes.props_radial    = capacity * sizeof(particle_radial_props);
        sizes.list            = capacity * sizeof(u32);
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
            sizes.position,
            sizes.velocity,
            sizes.props_parabolic,
            sizes.props_radial,
            sizes.list,
            sizes.list
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
        _particle_mngr->array_velocity =                 (vec3*)reservation_push_bytes(res, sizes.velocity);
        _particle_mngr->array_props_parabolic = (particle_parabolic_props*)reservation_push_bytes(res, sizes.props_parabolic);
        _particle_mngr->array_props_radial    =    (particle_radial_props*)reservation_push_bytes(res, sizes.props_radial);
        assert(_particle_mngr->array_hnd);
        assert(_particle_mngr->array_emitters);
        assert(_particle_mngr->array_life_ms);
        assert(_particle_mngr->array_colors);
        assert(_particle_mngr->array_position);
        assert(_particle_mngr->array_velocity);
        assert(_particle_mngr->array_props_parabolic);
        assert(_particle_mngr->array_props_radial);

        // every list starts empty
        for (particle_emitter_list& list : _particle_mngr->array_lists) {
            list.count       = 0;
            list.array_index = (u32*)reservation_push_bytes(res, sizes.list);
            assert(list.array_index);
        }

        // point each emitter at its slice of the particle arrays
        for (
            u32 emitter_index = 0;
                emitter_index < _particle_mngr->capacity;
              ++emitter_index) {

            const u32         particle_index = emitter_index * cfg.particle_bufffer_count;
            particle_emitter& emitter        = _particle_mngr->array_emitters[emitter_index];

            _particle_mngr->array_hnd[emitter_index] = INVALID_HANDLE;
            emitter.hnd            = INVALID_HANDLE;
            emitter.list_index     = INVALID_INDEX;
            emitter.array_life_ms  = &_particle_mngr->array_life_ms  [particle_index];
            emitter.array_colors   = &_particle_mngr->array_colors   [particle_index];
            emitter.array_position = &_particle_mngr->array_position [particle_index];
            emitter.array_velocity = &_particle_mngr->array_velocity [particle_index];
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

    IFB_INTERNAL particle_emitter_list&
    particle_mngr_get_list(
        const u32 type) {

        assert(_particle_mngr);
        assert(type < particle_emitter_type_e_count);

        auto& list = _particle_mngr->array_lists[type];
        return(list);
    }

    IFB_INTERNAL particle_parabolic_props&
    particle_mngr_get_props_parabolic(
        const u32 index) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_props_parabolic);
        assert(_particle_mngr->capacity > index);

        auto& props = _particle_mngr->array_props_parabolic[index];
        return(props);
    }

    IFB_INTERNAL particle_radial_props&
    particle_mngr_get_props_radial(
        const u32 index) {

        assert(_particle_mngr);
        assert(_particle_mngr->array_props_radial);
        assert(_particle_mngr->capacity > index);

        auto& props = _particle_mngr->array_props_radial[index];
        return(props);
    }

    IFB_INTERNAL void
    particle_mngr_activate_emitter(
        const u32 index) {

        particle_emitter& emitter = particle_mngr_get_emitter(index);
        if (emitter.list_index != INVALID_INDEX) return;

        // add the emitter to the end of the list for its type
        particle_emitter_list& list = particle_mngr_get_list(emitter.type);
        assert(list.count < _particle_mngr->capacity);

        emitter.list_index           = list.count;
        list.array_index[list.count] = index;
        ++list.count;
    }

    IFB_INTERNAL void
    particle_mngr_deactivate_emitter(
        const u32 index) {

        particle_emitter& emitter = particle_mngr_get_emitter(index);
        if (emitter.list_index == INVALID_INDEX) return;

        particle_emitter_list& list = particle_mngr_get_list(emitter.type);
        assert(list.count > emitter.list_index);
        assert(list.array_index[emitter.list_index] == index);

        // the last emitter in the list takes this one's place
        const u32 index_last = list.array_index[list.count - 1];
        list.array_index[emitter.list_index]             = index_last;
        particle_mngr_get_emitter(index_last).list_index = emitter.list_index;
        --list.count;

        emitter.list_index = INVALID_INDEX;
    }

};
