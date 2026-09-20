
#pragma once

#include "component.hpp"
#include "entity-lookup.cpp"
#include "entity-manager.cpp"
#include "entity.cpp"
#include "entity-component.cpp"
#include "ifb-entity.hpp"
#include "ifb-types.hpp"
#include "physics-entity.cpp"
#include "quad.cpp"
#include "renderer.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // LOOKUP 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool
    eng_atype_lookup_quad(
        const entity_id id,
        atype_quad&     quad) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, ENTITY_ARCHETYPE_QUAD);        
           
        if (result) {
            cmpnt_lookup_position   (e.index_sparse, quad.position);
            cmpnt_lookup_color      (e.index_sparse, quad.color);
            cmpnt_lookup_quad       (e.index_sparse, quad.quad);
            cmpnt_lookup_map_coords (e.index_sparse, quad.map_coords);
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
        result &= entity_has_component (e, ENTITY_ARCHETYPE_PHYSICS_PARTICLE);        
           
        if (result) {
            cmpnt_lookup_position     (e.index_sparse, particle.position);
            cmpnt_lookup_rigid_body   (e.index_sparse, particle.rigid_body);
            cmpnt_lookup_velocity     (e.index_sparse, particle.velocity);
            cmpnt_lookup_acceleration (e.index_sparse, particle.acceleration);
            cmpnt_lookup_inv_mass     (e.index_sparse, particle.inv_mass);
            cmpnt_lookup_drag         (e.index_sparse, particle.drag);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_atype_lookup_physics_quad(
        const entity_id     id,
        atype_physics_quad& physics_quad) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, ENTITY_ARCHETYPE_QUAD);        
           
        if (result) {
            cmpnt_lookup_position     (e.index_sparse, physics_quad.position);
            cmpnt_lookup_color        (e.index_sparse, physics_quad.color);
            cmpnt_lookup_quad         (e.index_sparse, physics_quad.quad);
            cmpnt_lookup_rigid_body   (e.index_sparse, physics_quad.rigid_body);
            cmpnt_lookup_velocity     (e.index_sparse, physics_quad.velocity);
            cmpnt_lookup_acceleration (e.index_sparse, physics_quad.acceleration);
            cmpnt_lookup_inv_mass     (e.index_sparse, physics_quad.inv_mass);
            cmpnt_lookup_drag         (e.index_sparse, physics_quad.drag);
            cmpnt_lookup_map_coords   (e.index_sparse, physics_quad.map_coords);
        }

        return(result);
    }

    //--------------------------------------------------------------------
    // UPDATE 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool
    eng_atype_update_quad(
        const entity_id   id,
        const atype_quad& quad) {
        
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, ENTITY_ARCHETYPE_QUAD); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position   (e.index_sparse, quad.position);
            cmpnt_update_color      (e.index_sparse, quad.color);
            cmpnt_update_quad       (e.index_sparse, quad.quad);
            cmpnt_update_map_coords (e.index_sparse, quad.map_coords);
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
        const bool has_component = entity_has_component (e, ENTITY_ARCHETYPE_QUAD); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position     (e.index_sparse, particle.position);
            cmpnt_update_rigid_body   (e.index_sparse, particle.rigid_body);
            cmpnt_update_velocity     (e.index_sparse, particle.velocity);
            cmpnt_update_acceleration (e.index_sparse, particle.acceleration);
            cmpnt_update_inv_mass     (e.index_sparse, particle.inv_mass);
            cmpnt_update_drag         (e.index_sparse, particle.drag);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_atype_update_physics_quad(
        const entity_id           id,
        const atype_physics_quad& physics_quad) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, ENTITY_ARCHETYPE_QUAD); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position     (e.index_sparse, physics_quad.position);
            cmpnt_update_color        (e.index_sparse, physics_quad.color);
            cmpnt_update_quad         (e.index_sparse, physics_quad.quad);
            cmpnt_update_rigid_body   (e.index_sparse, physics_quad.rigid_body);
            cmpnt_update_velocity     (e.index_sparse, physics_quad.velocity);
            cmpnt_update_acceleration (e.index_sparse, physics_quad.acceleration);
            cmpnt_update_inv_mass     (e.index_sparse, physics_quad.inv_mass);
            cmpnt_update_drag         (e.index_sparse, physics_quad.drag);
            cmpnt_update_map_coords   (e.index_sparse, physics_quad.map_coords);
        }

        return(can_update);
    }
};
