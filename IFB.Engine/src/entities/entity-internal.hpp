#ifndef ENTITY_INTERNAL_HPP
#define ENTITY_INTERNAL_HPP

#include "ifb-entities.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static entity_mngr* _entity_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL entity_mngr*     entity_mngr_create               (void);
    IFB_INTERNAL void             entity_mngr_validate             (void);
    IFB_INTERNAL void             entity_mngr_startup              (reservation* res);
    IFB_INTERNAL void             entity_mngr_shutdown             (void);
    IFB_INTERNAL u32              entity_mngr_capacity_dense       (void);
    IFB_INTERNAL u32              entity_mngr_capacity_sparse      (void);
    IFB_INTERNAL u32              entity_mngr_get_count            (void);

    // entity
    IFB_INTERNAL entity_id        entity_create                    (const cchar* tag_cstr);
    IFB_INTERNAL bool             entity_add_archetype             (const entity_id id, const entity_archetype atype);
    IFB_INTERNAL bool             entity_remove_archetype          (const entity_id id, const entity_archetype atype);
    IFB_INTERNAL bool             entity_destroy                   (const entity_id id);
    IFB_INTERNAL bool             entity_has_component             (const entity& e, const component_type type);

    // lookup
    IFB_INTERNAL bool             entity_lookup_list               (entity_list* list, const entity_query& query);
    IFB_INTERNAL bool             entity_lookup_by_tag             (entity& e, const cchar* tag_cstr);
    IFB_INTERNAL bool             entity_lookup_by_index_dense     (entity& e, const u32    index);
    IFB_INTERNAL bool             entity_lookup_by_id              (entity& e, const entity_id id);
    IFB_INTERNAL u32              entity_lookup_sparse_index       (const entity_id id);
    IFB_INTERNAL entity_archetype entity_lookup_archetype          (const entity_id id);
    IFB_INTERNAL const cchar*     entity_lookup_tag                (const entity_id id);

    // components
    IFB_INTERNAL bool             entity_component_add             (const entity_id id, const component_type types);
    IFB_INTERNAL bool             entity_component_remove          (const entity_id id, const component_type types);

    // testing
    IFB_INTERNAL void             entity_test (void);
};

#endif //ENTITY_INTERNAL_HPP
