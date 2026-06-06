#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include <sld.hpp>

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
        cchar8 cstr[ENTITY_TAG_SIZE];

        inline entity_tag& operator= (const entity_tag& other);
        inline entity_tag& operator= (const cchar8* cstr);
    };

    struct entity {
        entity_id         id;
        const entity_tag* tag;
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


#endif //IFB_ENTITY_HPP