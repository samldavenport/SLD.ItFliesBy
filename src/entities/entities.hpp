#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity;
    struct entity_mngr;
    struct entity_sparse_set;
    struct entity_batch;
    struct entity_list;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static entity_mngr* _entity_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL entity_mngr* entity_mngr_create           (void);
    IFB_INTERNAL void         entity_mngr_validate         (void);
    IFB_INTERNAL void         entity_mngr_startup          (const memory& mem_res);
    IFB_INTERNAL void         entity_mngr_shutdown         (void);

    // entity
    IFB_INTERNAL entity_id    entity_create                (const cchar* tag_cstr, const entity_archetype atype = component_type_e_none);
    IFB_INTERNAL bool         entity_destroy               (const cchar* tag_cstr);
    IFB_INTERNAL entity_list* entity_list_arena_create     (arena* a, const u32 count = 0);
    IFB_INTERNAL void         entity_list_validate         (const entity_list* list);
    IFB_INTERNAL bool         entity_lookup_by_archetype   (entity_list* id_list, const entity_archetype atype);
    IFB_INTERNAL bool         entity_lookup_by_tag         (entity& e, const cchar* tag_cstr);
    IFB_INTERNAL bool         entity_lookup_by_index_dense (entity& e, const u32    index);

    // testing
    IFB_INTERNAL void         entity_test                  (void);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_sparse_data {
        u32* dense_index;
        u32  capacity;
    };

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
        memory  mem;
        u32     count;
    };

    struct entity_list {
        struct {
            entity_id* id;
            u32*       sparse_index;
            u32*       dense_index;
        } data;
        u32        capacity;
        u32        count;
    };
};

#endif  //IFB_ENTITIES_HPP