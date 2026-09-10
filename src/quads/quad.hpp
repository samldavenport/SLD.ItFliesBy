#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "sld.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_vertex;
    struct quad_tile;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------


    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL bool       quad_does_exist     (const entity_id    id);
    IFB_INTERNAL void       quad_update         (const quad_entity& q);
    IFB_INTERNAL void       quad_update         (const entity_id id, const quad& q);

    IFB_INTERNAL void       quad_tests          (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack* stack_mem;
    };
};

#endif //IFB_QUAD_HPP
