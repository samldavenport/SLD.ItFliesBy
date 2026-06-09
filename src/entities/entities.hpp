#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 ENTITY_ID_INVALID = 0xFFFFFFFF;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity;
    struct entity_id;
    struct entity_tag;
    struct entity_manager;
    struct entity_sparse_array;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity id
    IFB_INTERNAL entity_id       entity_id_init                     (const entity_tag* tag);

    // entity tag
    IFB_INTERNAL void            entity_tag_init                    (entity_tag*       tag, const cchar8* cstr);
    IFB_INTERNAL u32             entity_tag_hash                    (const entity_tag* tag);

    // entity manager
    IFB_INTERNAL u32             entity_manager_memory_requirement  (void);
    IFB_INTERNAL entity_manager* entity_manager_memory_init         (const memory& mem);
    IFB_INTERNAL void            entity_manager_assert_valid        (const entity_manager* em);
    IFB_INTERNAL void            entity_manager_startup             (entity_manager*       em, const memory& mem_res);
    IFB_INTERNAL void            entity_manager_shutdown            (entity_manager*       em);

    // entity
    IFB_INTERNAL u32             entity_lookup_index_by_tag         (const entity_manager* em, const cchar8*    tag_cstr);
    IFB_INTERNAL u32             entity_lookup_index_by_id          (const entity_manager* em, const entity_id  id);
    IFB_INTERNAL void            entity_get                         (const entity_manager* em, const u32        index, entity* out);
    IFB_INTERNAL entity_id       entity_create                      (entity_manager*       em, const cchar8*    tag_cstr);
    IFB_INTERNAL bool            entity_destroy_by_tag              (entity_manager*       em, const cchar8*    tag_cstr);
    IFB_INTERNAL bool            entity_destroy_by_id               (entity_manager*       em, const entity_id& id);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_tag {
        cchar8 cstr[ENTITY_TAG_SIZE];

        inline entity_tag& operator= (const entity_tag& other);
        inline entity_tag& operator= (const cchar8* cstr);
    };

    struct entity {
        entity_id         id;
        const entity_tag* tag;
    };

    struct entity_manager {
        struct {
            entity_id*  id;
            entity_tag* tag;
            u32*        sparse_index;
        } data;
        memory  mem;
        u32     capacity;
        u32     count;
    };

    struct entity_sparse_array {
        struct {
            struct {
                entity_id* id;
                u32*       sparse_index;
            } dense;
            struct {
                u32* dense_index;
            } sparse;
        } data;
        u32 capacity_dense;
        u32 capacity_sparse;
        u32 count;
        f32 max_load_p100;
    };

    IFB_INTERNAL u32                  entity_sparse_array_memory_requirement (const u32 dense_capacity, const u32 max_load_p100);
    IFB_INTERNAL entity_sparse_array* entity_sparse_array_memory_init        (const u32 dense_capacity, const u32 max_load_p100, const u32 mem_size, void* mem_ptr);
    IFB_INTERNAL bool                 entity_sparse_array_insert             (entity_sparse_array* esa, const entity_id id);
    IFB_INTERNAL bool                 entity_sparse_array_remove             (entity_sparse_array* esa, const entity_id id);
    IFB_INTERNAL u32                  entity_sparse_array_lookup             (entity_sparse_array* esa, const entity_id id);

    //--------------------------------------------------------------------
    // ENTITY TAG INLINE OPERATORS
    //--------------------------------------------------------------------

    inline entity_tag&
    entity_tag::operator= (const entity_tag& other) {

        zero_memory((void*)this, sizeof(entity_tag));

        (void)memmove(
            (void*)&this->cstr[0],
            &other.cstr[0],
            ENTITY_TAG_SIZE
        );

        this->cstr[ENTITY_TAG_SIZE - 1] = 0;
        return(*this);
    }

    inline entity_tag&
    entity_tag::operator= (const cchar8* cstr) {

        zero_memory((void*)this, sizeof(entity_tag));

        const u32 cstr_len = strnlen_s(cstr, ENTITY_TAG_SIZE);

        (void)memmove(
            (void*)&this->cstr[0],
            (void*)cstr,
            cstr_len
        );

        this->cstr[ENTITY_TAG_SIZE - 1] = 0;
        return(*this);            
    }

};

#endif  //IFB_ENTITIES_HPP