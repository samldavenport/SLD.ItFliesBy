#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "entities.hpp"
namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct component_mngr;
    struct component_table_position;
    struct component_table_color;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static component_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL component_mngr* component_mngr_create  (void);
    IFB_INTERNAL void            component_mngr_startup (memory& mem_res);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct component_mngr {
        stack mem;
        u32   capacity;
        struct  {
            component_table_position* position;
            component_table_color*    color; 
        } tables;
    };

    struct component_table_position {
        f32* x;
        f32* y;
        f32* z;
    };

    struct component_table_color {
        u32* rgba_hex;
    };
};

#endif //COMPONENTS_HPP