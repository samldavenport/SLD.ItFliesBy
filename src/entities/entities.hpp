#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"

namespace ifb {

    constexpr u32 ENTITY_ID_INVALID = 0xFFFFFFFF;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    struct entity;
    struct entity_id;
    struct entity_tag;
    struct entity_manager;
    
    // entity id
    entity_id     entity_id_init                   (const entity_tag* tag);

    // entity tag
    void          entity_tag_init                  (entity_tag*       tag, const cchar8* cstr);
    u32           entity_tag_hash                  (const entity_tag* tag);

    // entity table
    u32             entity_manager_memory_requirement  (const u32 capacity);
    entity_manager* entity_manager_memory_init         (const u32 capacity, const u32 mem_size, void* mem_ptr);
    void            entity_manager_assert_valid        (const entity_manager* em);
    u32             entity_manager_lookup_index_by_tag (const entity_manager* em, const cchar8*    tag_cstr);
    u32             entity_manager_lookup_index_by_id  (const entity_manager* em, const entity_id  id);
    void            entity_manager_get                 (const entity_manager* em, const u32        index, entity* out);
    entity_id       entity_manager_insert              (entity_manager*       em, const cchar8*    tag_cstr);
    bool            entity_manager_remove_by_tag       (entity_manager*       em, const cchar8*    tag_cstr);
    bool            entity_manager_remove_by_id        (entity_manager*       em, const entity_id* id);

    struct entity_id {
        u32 hash;

        inline bool
        operator==(
            const entity_id& other) const {

            return(this->hash == other.hash);
        }

        inline bool
        operator!=(
            const entity_id& other) const {

            return(this->hash != other.hash);
        }

        inline bool
        operator==(
            const u32& hash) const {

            return(this->hash == hash);
        }

        inline bool
        operator!=(
            const u32& hash) const {

            return(this->hash != hash);
        }

        inline entity_id&
        operator= (
            const u32 hash) {

            this->hash = hash;
            return(*this);
        }

        inline entity_id&
        operator= (
            const entity_id& id) {

            this->hash = id.hash;
            return(*this);
        }
    };

    struct entity_tag {
        cchar8 cstr[ENTITY_TAG_SIZE];

        inline entity_tag&
        operator= (const entity_tag& other) {

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
        operator= (const cchar8* cstr) {

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

    struct entity {
        entity_id         id;
        const entity_tag* tag;
    };

    struct entity_manager {
        struct {
            entity_id*  id;
            entity_tag* tag;
        } data;
        u32     capacity;
        u32     count;
    };
};

#endif  //IFB_ENTITIES_HPP