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

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    static entity_manager* _entity_manager;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity id
    IFB_INTERNAL entity_id       entity_id_init                     (const entity_tag* tag);

    // entity tag
    IFB_INTERNAL void            entity_tag_init                    (entity_tag*       tag, const cchar* cstr);
    IFB_INTERNAL u32             entity_tag_hash                    (const entity_tag* tag);

    // entity manager
    IFB_INTERNAL u32             entity_manager_memory_requirement  (void);
    IFB_INTERNAL entity_manager* entity_manager_memory_init         (const memory& mem);
    IFB_INTERNAL void            entity_manager_assert_valid        (void);
    IFB_INTERNAL void            entity_manager_startup             (const memory& mem_res);
    IFB_INTERNAL void            entity_manager_shutdown            (void);

    // entity
    IFB_INTERNAL u32             entity_lookup_index_by_tag         (const cchar*    tag_cstr);
    IFB_INTERNAL u32             entity_lookup_index_by_id          (const entity_id  id);
    IFB_INTERNAL void            entity_get                         (const u32        index, entity* out);
    IFB_INTERNAL entity_id       entity_create                      (const cchar*    tag_cstr);
    IFB_INTERNAL bool            entity_destroy_by_tag              (const cchar*    tag_cstr);
    IFB_INTERNAL bool            entity_destroy_by_id               (const entity_id& id);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_id {
        u32 hash;

        inline bool       operator== (const entity_id& other) const;
        inline bool       operator!= (const entity_id& other) const;
        inline bool       operator== (const u32& hash)        const;
        inline bool       operator!= (const u32& hash)        const;
        inline entity_id& operator=  (const u32& hash);
        inline entity_id& operator=  (const entity_id& id);
    };
    
    struct entity_tag {
        cchar cstr[ENTITY_TAG_SIZE];

        inline entity_tag& operator= (const entity_tag& other);
        inline entity_tag& operator= (const cchar* cstr);
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
        memory  mem;
        u32     capacity;
        u32     count;
    };

    //--------------------------------------------------------------------
    // ENTITY ID INLINE OPERATORS
    //--------------------------------------------------------------------

    inline bool
    entity_id::operator==(
        const entity_id& other) const {

        return(this->hash == other.hash);
    }

    inline bool
    entity_id::operator!=(
        const entity_id& other) const {

        return(this->hash != other.hash);
    }

    inline bool
    entity_id::operator==(
        const u32& hash) const {

        return(this->hash == hash);
    }

    inline bool
    entity_id::operator!=(
        const u32& hash) const {

        return(this->hash != hash);
    }

    inline entity_id&
    entity_id::operator=(
        const u32& hash) {

        this->hash = hash;
        return(*this);
    }

    inline entity_id&
    entity_id::operator= (
        const entity_id& id) {

        this->hash = id.hash;
        return(*this);
    }

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
    entity_tag::operator= (const cchar* cstr) {

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