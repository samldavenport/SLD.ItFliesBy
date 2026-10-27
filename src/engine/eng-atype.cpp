
#pragma once

#include "component.hpp"
#include "entity.cpp"
#include "ifb-entity.hpp"
#include "ifb-types.hpp"
#include "map.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // LOOKUP 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool
    eng_atype_lookup_sprite(
        const entity_id id,
        atype_sprite&     quad) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= e.archetype.has_all(ENTITY_ARCHETYPE_SPRITE);        
           
        if (result) {
            cmpnt_lookup_position    (e.index_sparse, quad.position);
            cmpnt_lookup_color       (e.index_sparse, quad.color);
            cmpnt_lookup_quad        (e.index_sparse, quad.quad);
            map_entity_lookup_coords (e, quad.map_coords);
            cmpnt_lookup_sprite      (e.index_sparse, quad.sprite);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_atype_lookup_particle(
        const entity_id id,
        atype_particle& particle) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result &= entity_lookup_by_id  (e, id);
        result &= e.archetype.has_all(ENTITY_ARCHETYPE_PHYSICS_PARTICLE);        
           
        if (result) {
            cmpnt_lookup_position      (e.index_sparse, particle.position);
            cmpnt_lookup_velocity      (e.index_sparse, particle.velocity);
            cmpnt_lookup_acceleration  (e.index_sparse, particle.acceleration);
            cmpnt_lookup_inv_mass      (e.index_sparse, particle.inv_mass);
            cmpnt_lookup_drag          (e.index_sparse, particle.drag);
            cmpnt_lookup_term_velocity (e.index_sparse, particle.term_velocity);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_atype_lookup_physics_quad(
        const entity_id     id,
        atype_physics_sprite& physics_quad) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= e.archetype.has_all(ENTITY_ARCHETYPE_PHYSICS_SPRITE);        
           
        if (result) {
            cmpnt_lookup_position     (e.index_sparse, physics_quad.position);
            cmpnt_lookup_color        (e.index_sparse, physics_quad.color);
            cmpnt_lookup_quad         (e.index_sparse, physics_quad.quad);
            cmpnt_lookup_velocity     (e.index_sparse, physics_quad.velocity);
            cmpnt_lookup_acceleration (e.index_sparse, physics_quad.acceleration);
            cmpnt_lookup_inv_mass     (e.index_sparse, physics_quad.inv_mass);
            cmpnt_lookup_drag         (e.index_sparse, physics_quad.drag);
            cmpnt_lookup_term_velocity(e.index_sparse, physics_quad.term_velocity);
            map_entity_lookup_coords  (e, physics_quad.map_coords);
        }

        return(result);
    }

    //--------------------------------------------------------------------
    // UPDATE 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool
    eng_atype_update_sprite(
        const entity_id     id,
        const atype_sprite& sprite) {
        
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = e.archetype.has_all(ENTITY_ARCHETYPE_SPRITE); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position   (e.index_sparse, sprite.position);
            cmpnt_update_color      (e.index_sparse, sprite.color);
            cmpnt_update_quad       (e.index_sparse, sprite.quad);
            cmpnt_update_map_coords (e.index_sparse, sprite.map_coords);
            cmpnt_update_sprite     (e.index_sparse, sprite.sprite);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_atype_update_particle(
        const entity_id       id,
        const atype_particle& particle) {
        
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = e.archetype.has_all(ENTITY_ARCHETYPE_PHYSICS_PARTICLE); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position     (e.index_sparse, particle.position);
            cmpnt_update_velocity     (e.index_sparse, particle.velocity);
            cmpnt_update_acceleration (e.index_sparse, particle.acceleration);
            cmpnt_update_inv_mass     (e.index_sparse, particle.inv_mass);
            cmpnt_update_drag         (e.index_sparse, particle.drag);
            cmpnt_update_term_velocity(e.index_sparse, particle.term_velocity);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_atype_update_physics_sprite(
        const entity_id             id,
        const atype_physics_sprite& physics_sprite) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = e.archetype.has_all(ENTITY_ARCHETYPE_PHYSICS_SPRITE); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position     (e.index_sparse, physics_sprite.position);
            cmpnt_update_color        (e.index_sparse, physics_sprite.color);
            cmpnt_update_quad         (e.index_sparse, physics_sprite.quad);
            cmpnt_update_velocity     (e.index_sparse, physics_sprite.velocity);
            cmpnt_update_acceleration (e.index_sparse, physics_sprite.acceleration);
            cmpnt_update_inv_mass     (e.index_sparse, physics_sprite.inv_mass);
            cmpnt_update_drag         (e.index_sparse, physics_sprite.drag);
            cmpnt_update_map_coords   (e.index_sparse, physics_sprite.map_coords);
            cmpnt_update_term_velocity(e.index_sparse, physics_sprite.term_velocity);
            cmpnt_update_sprite       (e.index_sparse, physics_sprite.sprite);
        }

        return(can_update);
    }
};
