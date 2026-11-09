#ifndef ENG_ENTITY_HPP
#define ENG_ENTITY_HPP

#include "ifb.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPES 
    //--------------------------------------------------------------------

    struct entity_mngr {
        struct {
            struct {
                entity_id* id;
                tag*       tag;
                archetype* archetype;
                u32*       sparse_index;
            } dense;
            struct {
                u32* dense_index;
            } sparse;
        } data;
        struct {
            u32 dense;
            u32 sparse;
        } capacity;
        reservation* res;
        u32          count;
    } static * _entity_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL entity_mngr* entity_mngr_create               (void);
    IFB_INTERNAL void         entity_mngr_validate             (void);
    IFB_INTERNAL void         entity_mngr_startup              (reservation* res);
    IFB_INTERNAL void         entity_mngr_shutdown             (void);
    IFB_INTERNAL u32          entity_mngr_capacity_dense       (void);
    IFB_INTERNAL u32          entity_mngr_capacity_sparse      (void);
    IFB_INTERNAL u32          entity_mngr_get_count            (void);

    // entity
    IFB_INTERNAL entity_id    entity_create                    (const cchar* tag_cstr);
    IFB_INTERNAL bool         entity_add_archetype             (const entity_id id, const archetype atype);
    IFB_INTERNAL bool         entity_remove_archetype          (const entity_id id, const archetype atype);
    IFB_INTERNAL bool         entity_destroy                   (const entity_id id);
    IFB_INTERNAL bool         entity_has_component             (const entity& e, const cmpnt_type_u32 type);

    // lookup
    IFB_INTERNAL bool         entity_lookup_list               (entity_list* list, const entity_query& query);
    IFB_INTERNAL bool         entity_lookup_by_tag             (entity& e, const cchar* tag_cstr);
    IFB_INTERNAL bool         entity_lookup_by_index_dense     (entity& e, const u32    index);
    IFB_INTERNAL bool         entity_lookup_by_id              (entity& e, const entity_id id);
    IFB_INTERNAL u32          entity_lookup_sparse_index       (const entity_id id);
    IFB_INTERNAL archetype    entity_lookup_archetype          (const entity_id id);
    IFB_INTERNAL const cchar* entity_lookup_tag                (const entity_id id);

    // components
    IFB_INTERNAL bool         entity_component_add             (const entity_id id, const cmpnt_type_u32 types);
    IFB_INTERNAL bool         entity_component_remove          (const entity_id id, const cmpnt_type_u32 types);
};

#endif //ENG_ENTITY_HPP
