#ifndef ENG_ENTITY_HPP
#define ENG_ENTITY_HPP

#include "ifb.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY TAG
    //--------------------------------------------------------------------

    struct entity_tag {

        cchar cstr[ENTITY_TAG_SIZE];

        entity_tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);
            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);

            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline entity_id
        to_id(void) const {

            const entity_id id = hash_u32((void*)this->cstr, ENTITY_TAG_SIZE);
            return(id);
        }
    };

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity_mngr;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_mngr {
        struct {
            struct {
                entity_id*        id;
                entity_tag*       tag;
                entity_archetype* archetype;
                u32*              sparse_index;
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
    };

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

#endif //ENG_ENTITY_HPP
