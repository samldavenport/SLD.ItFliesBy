#pragma once

#include "component.hpp"
#include "entity-lookup.cpp"
#include "entity-manager.cpp"
#include "entity.cpp"
#include "entity-component.cpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "physics-entity.cpp"
#include "quad.cpp"
#include "ifb-renderer.hpp"
#include "sprites.hpp"
#include <cassert>
#include "entity-internal.hpp"
#include "physics-internal.hpp"
#include "quad-internal.hpp"
#include "renderer-internal.hpp"

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
    eng_entity_lookup_by_id(
        const entity_id id,
        entity&         e) {

        const bool did_find = entity_lookup_by_id(e, id); 
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
        bool has_component = e.archetype.has_all(
                cmpnt_type_e_quad  |
                cmpnt_type_e_color |
                cmpnt_type_e_position
        ); 

        const bool can_render = (does_exist && has_component); 
        const bool did_push   = can_render && renderer_push_quad(id);

        // an entity with a particle emitter renders its particles too
        const bool has_emitter      = does_exist  && e.archetype.has_all(cmpnt_type_e_position | cmpnt_type_e_particle_emitter);
        const bool did_push_emitter = has_emitter && renderer_push_particle_emitter(id);

        return(did_push || did_push_emitter);
    }

    IFB_ENGINE_API bool
    eng_entity_add_force(
        const entity_id id,
        const vec3&     force) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = e.archetype.has_all(
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
    
    IFB_ENGINE_API bool
    eng_entity_set_sprite(
        const entity_id id,
        const cchar*    atlas_name_cstr,
        const cchar*    sprite_name_cstr) {

        assert(id               != ENTITY_ID_INVALID);
        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        if (!entity_component_add(id, cmpnt_type_e_sprite)) {
            return(false);
        }

        cmpnt_sprite sprite;
        sprite.h_atlas  = sprite_mngr_lookup_atlas        (atlas_name_cstr); 
        sprite.i_sprite = sprite_mngr_lookup_sprite_index (sprite.h_atlas, sprite_name_cstr);
    
        const bool is_valid = (
           sprite.h_atlas  != INVALID_HANDLE &&
           sprite.i_sprite != INVALID_INDEX
        );

        if (is_valid) {
            // we know its a valid entity with an explicitly added sprite component
            // if it doesn't update, something went VERY wrong
            const bool did_update = eng_cmpnt_update_sprite(id, sprite);
            assert(did_update);
        }

        return(is_valid);
    }
};
