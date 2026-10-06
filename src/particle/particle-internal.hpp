#ifndef PARTICLE_INTERNAL_HPP
#define PARTICLE_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "particle.hpp"
#include "sld.hpp"
#include <sld-strings.hpp>

namespace ifb {

    struct particle_emitter {
        hnd_particle_emitter hnd;
        u32                  count;
        f32                  lifespan_ms;
        f32                  height;
        f32                  width;
        color_rgba_u32       color_primary;
        color_rgba_u32       color_secondary;
        f32*                 array_life_ms;
        color_rgba_u32*      array_colors;
        vec3*                array_position;
        cstr_c16             name;
    };  

    IFB_INTERNAL particle_emitter& particle_mngr_get_emitter  (const u32 index);
    IFB_INTERNAL particle_emitter& particle_mngr_set_handle   (const u32 index, const hnd_particle_emitter);
    IFB_INTERNAL u32               particle_mngr_find_emitter_index_existing (const hnd_particle_emitter h_emitter);
    IFB_INTERNAL u32               particle_mngr_find_emitter_index_new      (const hnd_particle_emitter h_emitter);
};

#endif //PARTICLE_INTERNAL_HPP
