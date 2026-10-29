#ifndef COMPONENT_INTERNAL_HPP
#define COMPONENT_INTERNAL_HPP

#include "ifb.hpp"
#include "sld.hpp"
#include "component.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------
    
    struct cmpnt_table;
    
    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;
    
    //--------------------------------------------------------------------
    // TABLE DEFINITIONS
    //--------------------------------------------------------------------

    struct cmpnt_table {
        cmpnt_position*         array_position; 
        cmpnt_color*            array_color;
        cmpnt_quad*             array_quad; 
        cmpnt_velocity*         array_velocity; 
        cmpnt_acceleration*     array_acceleration;
        cmpnt_inv_mass*         array_inv_mass;
        cmpnt_drag*             array_drag;
        cmpnt_term_velocity*    array_term_velocity; 
        cmpnt_spring*           array_spring; 
        cmpnt_map_coords*       array_map_coords;
        cmpnt_particle_emitter* array_particle_emitter;
        cmpnt_sprite*           array_sprite;
    };
    
    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        reservation* res;
        cmpnt_table* cmpnt_tbl;
        u32          capacity;
    };
};
#endif //COMPONENT_INTERNAL_HPP
