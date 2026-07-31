#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"
#include "entity.cpp"

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

        return(false);    
    }

    IFB_ENGINE_API bool
    eng_entity_destroy_by_tag(
        const cchar* tag) {
    
        assert(tag);

        const bool did_destroy = entity_destroy(tag);
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

    IFB_ENGINE_API const bool
    eng_entity_add_components(
        const entity_id      id,
        const component_type type) {
   
        assert(id != ENTITY_ID_INVALID);
        
        const bool did_add = entity_component_add(id, type);
        return(did_add);
    }

    IFB_ENGINE_API const bool
    eng_entity_remove_components(
        const entity_id      id,
        const component_type type) {
    
        assert(id != ENTITY_ID_INVALID); 

        const bool did_remove = entity_component_remove(id, type);
        return(did_remove);
    }
    IFB_ENGINE_API const bool   eng_entity_update_position     (const entity_id id, const position_3d&     pos);
    IFB_ENGINE_API const bool   eng_entity_update_velocity     (const entity_id id, const velocity_3d&     vel);
    IFB_ENGINE_API const bool   eng_entity_update_acceleration (const entity_id id, const acceleration_3d& acc);
    IFB_ENGINE_API const bool   eng_entity_update_inv_mass     (const entity_id id, const f32              inv_mass);
    IFB_ENGINE_API const bool   eng_entity_update_mass         (const entity_id id, const f32              mass);
    IFB_ENGINE_API const bool   eng_entity_update_drag         (const entity_id id, const f32              drag);
    IFB_ENGINE_API const bool   eng_entity_update_quad         (const entity_id id, const quad&            quad);
    IFB_ENGINE_API const bool   eng_entity_update_color        (const entity_id id, const color_rgba_u32&  color);
    IFB_ENGINE_API const bool   eng_entity_render              (const entity_id id);
    IFB_ENGINE_API const bool   eng_entity_add_force           (const entity_id id, const vec3& force);
};
