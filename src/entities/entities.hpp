#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"


namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity;
    struct entity_manager;
    struct entity_sparse_set;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    static entity_manager* _entity_manager;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL u32                entity_manager_memory_requirement    (void);
    IFB_INTERNAL entity_manager*    entity_manager_memory_init           (const memory& mem);
    IFB_INTERNAL void               entity_manager_assert_valid          (void);
    IFB_INTERNAL void               entity_manager_startup               (const memory& mem_res);
    IFB_INTERNAL void               entity_manager_shutdown              (void);

    // entity
    IFB_INTERNAL u32                entity_lookup_index_by_tag           (const cchar*    tag_cstr);
    IFB_INTERNAL u32                entity_lookup_index_by_id            (const entity_id id);
    IFB_INTERNAL void               entity_get                           (const u32       index, entity* out);
    IFB_INTERNAL entity_id          entity_create                        (const cchar*    tag_cstr);
    IFB_INTERNAL bool               entity_destroy_by_tag                (const cchar*    tag_cstr);
    IFB_INTERNAL bool               entity_destroy_by_id                 (const entity_id id);

    // entity sparse set
    IFB_INTERNAL u32                entity_sparse_set_memory_requirement (const u32 capacity);
    IFB_INTERNAL entity_sparse_set* entity_sparse_set_memory_init        (const u32 capacity, memory& mem);
    IFB_INTERNAL void               entity_sparse_set_validate           (const entity_sparse_set* ess);
    IFB_INTERNAL u32                entity_sparse_set_capacity_sparse    (const entity_sparse_set* ess);
    IFB_INTERNAL u32                entity_sparse_set_capacity_dense     (const entity_sparse_set* ess);
    IFB_INTERNAL bool               entity_sparse_set_index_of           (const entity_sparse_set* ess, const cchar* tag_cstr, u32& index);
    IFB_INTERNAL void               entity_sparse_set_reset              (entity_sparse_set* ess);
    IFB_INTERNAL void               entity_sparse_set_remove             (entity_sparse_set* ess, const cchar* tag_cstr);
    IFB_INTERNAL u32                entity_sparse_set_insert             (entity_sparse_set* ess, const cchar* tag_cstr);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct entity_manager {
        struct {
            entity_id*        id;
            entity_tag*       tag;
            entity_archetype* archetype;
        } data;
        memory  mem;
        u32     capacity;
        u32     count;
    };
};

#endif  //IFB_ENTITIES_HPP