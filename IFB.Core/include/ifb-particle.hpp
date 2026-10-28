#ifndef IFB_PARTICLE_HPP
#define IFB_PARTICLE_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-config.hpp"
#include "ifb-memory.hpp"
#include "sld.hpp"

namespace ifb {
     
    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
    };

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------

    struct particle_mngr;
    struct particle_buffer;
    struct particle_vertex;

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct particle_buffer {
        u32       count;
        particle* particles;
    };

    struct particle_vertex {
        vec3           position;
        color_rgba_u32 color;
    };
};

#endif //IFB_PARTICLE_HPP
