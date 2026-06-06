#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity_manager;

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

    struct entity_manager {
        struct {
            entity_id*  id;
            entity_tag* tag;
        } data;
        memory  mem;
        u32     capacity;
        u32     count;
    };

};

#endif  //IFB_ENTITIES_HPP