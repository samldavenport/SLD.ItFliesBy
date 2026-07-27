#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb.hpp"
#include "entity.hpp"
#include "component.hpp"
#include "memory.hpp"
#include "collections-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_mngr;
    struct quad_entity;
    struct quad_vertex;


    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static const entity_archetype QUAD_ENTITY_ARCHETYPE = (
        cmpnt_type_e_position |
        cmpnt_type_e_color    |
        cmpnt_type_e_quad
    );

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

    IFB_INTERNAL entity_id  quad_create         (const cchar*  tag_cstr);
    IFB_INTERNAL entity_id  quad_create         (const cchar*  tag_cstr, quad q);
    IFB_INTERNAL void       quad_create_batch   (const cchar** tag_cstr, const u32 count, entity_id* id);
    IFB_INTERNAL bool       quad_lookup_by_tag  (quad_entity& q, const cchar*    tag_cstr);
    IFB_INTERNAL bool       quad_lookup_by_id   (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool       quad_does_exist     (const entity_id    id);
    IFB_INTERNAL void       quad_update         (const quad_entity& q);
    IFB_INTERNAL void       quad_lookup_all     (entity_id_list&    ql);

    IFB_INTERNAL void       quad_tests          (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack          stack_mem;
        entity_id_list quad_id_list;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };
};

#endif //IFB_QUAD_HPP
