#pragma once

#include "component.hpp"
#include "entity-lookup.cpp"
#include "entity-manager.cpp"
#include "entity.cpp"
#include "entity-component.cpp"
#include "ifb-types.hpp"
#include "physics-entity.cpp"
#include "quad.cpp"
#include "renderer.hpp"
#include <cassert>

namespace ifb {
    
    IFB_ENGINE_API entity_id
    eng_entity_create(
        const cchar* tag) {

        assert(tag != NULL);

        const entity_id id = entity_create(tag);

        return(id);
    }
    
    IFB_ENGINE_API bool
    eng_entity_destroy_by_id(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        bool did_destroy = entity_destroy(id);
        return(did_destroy);
    }

    IFB_ENGINE_API u32
    eng_entity_get_count(
        void) {

        return(entity_mngr_get_count());
    }

    IFB_ENGINE_API bool
    eng_entity_destroy_by_tag(
        const cchar* tag) {
    
        assert(tag);

        bool did_destroy = entity_destroy(tag);
        return(did_destroy);
    }

    IFB_ENGINE_API const cchar*
    eng_entity_get_tag(
        const entity_id id) {
    
        assert(id != ENTITY_ID_INVALID);

        entity e;

        const cchar* tag = entity_lookup_by_id(e, id)
            ? e.tag
            : NULL;

        return(tag);
    }

    IFB_ENGINE_API bool
    eng_entity_add_components(
        const entity_id      id,
        const component_type type) {
   
        assert(id != ENTITY_ID_INVALID);
        
        bool did_add = entity_component_add(id, type);
        return(did_add);
    }

    IFB_ENGINE_API bool
    eng_entity_remove_components(
        const entity_id      id,
        const component_type type) {
    
        assert(id != ENTITY_ID_INVALID); 

        bool did_remove = entity_component_remove(id, type);
        return(did_remove);
    }

    IFB_ENGINE_API bool 
    eng_entity_lookup_by_dense_index(
        const u32 dense_index,
        entity&   e) {

        bool did_find = entity_lookup_by_index_dense(e, dense_index);
        return(did_find);
    }
    
    IFB_ENGINE_API bool
    eng_entity_query(
        entity_list*       list,
        const entity_query query) {

        assert(list);
        bool result = entity_lookup_list(list, query);
        return(result);
    }
    
    IFB_ENGINE_API bool
    eng_entity_quad_lookup(
        quad_entity& q,
        const entity_id id) {

        bool result = quad_lookup_by_id(q, id);
        return(result);
    }
    
    IFB_ENGINE_API bool
    eng_entity_render(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e,
                cmpnt_type_e_quad  |
                cmpnt_type_e_color |
                cmpnt_type_e_position
            ); 
        const bool can_render    = (does_exist && has_component); 

        if (can_render) {
            renderer_quad_push(id);
        }

        return(can_render);

    }

    IFB_ENGINE_API bool
    eng_entity_add_force(
        const entity_id id,
        const vec3&     force) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e,
           cmpnt_type_e_position     |
           cmpnt_type_e_velocity     |
           cmpnt_type_e_acceleration |
           cmpnt_type_e_inv_mass     |
           cmpnt_type_e_drag
        ); 
        const bool can_add_force = (does_exist && has_component); 

        if (can_add_force) {
            phys_entity_add_force(id, force);
        }

        return(can_add_force);
    }
};
