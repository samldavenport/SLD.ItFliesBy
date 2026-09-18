#ifndef COMPONENT_INTERNAL_HPP
#define COMPONENT_INTERNAL_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "memory.hpp"
#include "sld.hpp"
#include "component.hpp"

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
        cmpnt_position*      array_position; 
        color_rgba_u32*   array_color;
        quad*             array_quad; 
        rigid_body*       array_rigid_body;
        cmpnt_velocity*      array_velocity; 
        cmpnt_acceleration*  array_acceleration;
        inv_mass*         array_inv_mass;
        drag*             array_drag;
        term_cmpnt_velocity* array_term_velocity; 
        spring*           array_spring; 
        map_coords*       array_map_coords; 
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
