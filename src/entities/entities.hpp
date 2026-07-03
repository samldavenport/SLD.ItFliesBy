#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity;
    struct entity_manager;
    struct entity_sparse_set;
    struct entity_batch;
    struct entity_id_list;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    static entity_manager* _entity_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL u32                entity_mngr_memory_requirement (void);
    IFB_INTERNAL entity_manager*    entity_mngr_memory_init        (const memory& mem);
    IFB_INTERNAL void               entity_mngr_validate           (void);
    IFB_INTERNAL void               entity_mngr_startup            (const memory& mem_res);
    IFB_INTERNAL void               entity_mngr_shutdown           (void);

    // entity
    IFB_INTERNAL bool               entity_lookup_by_archetype     (entity_id_list* id_list, const entity_archetype atype);
    IFB_INTERNAL entity_id          entity_create                  (const cchar*    tag_cstr, const entity_archetype atype = component_type_e_none);
    IFB_INTERNAL bool               entity_destroy_by_tag          (const cchar*    tag_cstr);
    IFB_INTERNAL bool               entity_destroy_by_id           (const entity_id id);
    IFB_INTERNAL entity_id_list*    entity_id_list_arena_create    (arena* a);
    IFB_INTERNAL void               entity_id_list_validate        (const entity_id_list* list);

    // entity sparse set
    IFB_INTERNAL u32                entity_sset_memory_requirement (const u32 capacity);
    IFB_INTERNAL entity_sparse_set* entity_sset_memory_init        (const u32 capacity, memory& mem);
    IFB_INTERNAL void               entity_sset_validate           (const entity_sparse_set* ess);
    IFB_INTERNAL u32                entity_sset_capacity_sparse    (const entity_sparse_set* ess);
    IFB_INTERNAL u32                entity_sset_capacity_dense     (const entity_sparse_set* ess);
    IFB_INTERNAL bool               entity_sset_index_of           (const entity_sparse_set* ess, const cchar* tag_cstr, u32& index);
    IFB_INTERNAL void               entity_sset_reset              (entity_sparse_set* ess);
    IFB_INTERNAL void               entity_sset_remove             (entity_sparse_set* ess, const cchar* tag_cstr);
    IFB_INTERNAL u32                entity_sset_insert             (entity_sparse_set* ess, const cchar* tag_cstr);

    // testing
    IFB_INTERNAL void               entity_test                    (void);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_sparse_data {
        u32* dense_index;
        u32  capacity;
    };

    struct entity_manager {

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


    struct entity_sset {
        struct {
        
            struct {
                entity_id* id;
                u32*       sparse_index;
            } dense;
        
            struct {
                entity_tag*       tag;
                entity_archetype* archetype;
                u32*              dense_index;
            } sparse;
        
        } data;
        
    };

    struct entity_id_list {
        entity_id* array;
        u32        capacity;
        u32        count;
    };
};

#endif  //IFB_ENTITIES_HPP