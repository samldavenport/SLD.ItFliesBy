#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-components.hpp"

namespace ifb {

    constexpr u32 ENTITY_ID_INVALID = 0xFFFFFFFF;
    constexpr u32 ENTITY_TAG_SIZE   = 16;


    struct entity_tag {
        cchar cstr[ENTITY_TAG_SIZE];

        entity_tag() = default;
        entity_tag(
            const cchar* tag_cstr) {

            assert(tag_cstr != NULL);

            const u32 len = strnlen_s(tag_cstr, ENTITY_TAG_SIZE);
            (void)strncpy(cstr, tag_cstr, len);
        }
    };

    struct entity_id {

        u32 hash;

        entity_id() = default;
        entity_id(u32 h) : hash(h) { }
        entity_id(const entity_tag& tag) {}

        inline const u32 val(void) { return(hash); }
        inline bool operator== (const entity_id& other) const { return(hash == other.hash); }
        inline bool operator!= (const entity_id& other) const { return(hash != other.hash); }
        inline bool operator== (const u32& hash)        const { return(hash == hash);       }
        inline bool operator!= (const u32& hash)        const { return(hash != hash);       }
        inline u32  operator&  (const u32  val)         const { return(val  & hash);        }
    };
    inline u32 operator& (const u32 val, const entity_id id) { return(val & id.hash);}


    struct entity_archetype {

        u32 component_mask;

        entity_archetype() = default;
        entity_archetype(u32 mask) : component_mask(mask) { }

        inline bool operator== (const entity_archetype& other) const { return(component_mask == other.component_mask); }
        inline bool operator!= (const entity_archetype& other) const { return(component_mask != other.component_mask); }
        inline u32  operator&  (const entity_archetype& other) const { return(component_mask &  other.component_mask); }
        inline u32  operator|  (const entity_archetype& other) const { return(component_mask |  other.component_mask); }
        inline void operator&= (const entity_archetype& other)       { component_mask &= other.component_mask;         }
        inline void operator|= (const entity_archetype& other)       { component_mask |= other.component_mask;         }
        inline bool operator== (const u32 mask)                const { return(component_mask == mask);                 }
        inline bool operator!= (const u32 mask)                const { return(component_mask != mask);                 }
        inline u32  operator&  (const u32 mask)                const { return(component_mask &  mask);                 }
        inline u32  operator|  (const u32 mask)                const { return(component_mask |  mask);                 }
        inline void operator&= (const u32 mask)                      { component_mask &= mask;                         }
        inline void operator|= (const u32 mask)                      { component_mask |= mask;                         }
    };

    class entity {
    
    private:
        entity_id         id;
        entity_archetype  archetype;
        entity_tag        tag;

    public:

        inline entity() = default;

        inline 
        entity(const cchar* tag_cstr, const entity_archetype = component_type_e_none) {
            tag = entity_tag (tag_cstr);
            id  = entity_id  (tag);
        } 

        inline
        entity(
            const entity_id   id,
            const entity_tag& tag) {

            this->id  = id;
            this->tag = tag;
        }

        inline entity&
        add_component(
            const component_type type) {

            archetype |= type.val;
            return(*this);
        }

        inline const cchar*
        tag_cstr(void) {

            return(tag.cstr);
        }
    };
};


#endif //IFB_ENTITY_HPP