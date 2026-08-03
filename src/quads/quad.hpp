#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "collections-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_mngr;
    struct quad_entity;
    struct quad_vertex;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static quad_mngr* _quad_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL quad_mngr* quad_mngr_create    (void);
    IFB_INTERNAL void       quad_mngr_validate  (void);
    IFB_INTERNAL void       quad_mngr_startup   (memory& mem);

    IFB_INTERNAL bool       quad_lookup_by_id   (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool       quad_does_exist     (const entity_id    id);
    IFB_INTERNAL void       quad_update         (const quad_entity& q);
    IFB_INTERNAL void       quad_update         (const entity_id id, const quad& q);

    IFB_INTERNAL void       quad_tests          (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack          stack_mem;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };
};

#endif //IFB_QUAD_HPP
