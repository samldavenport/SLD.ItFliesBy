#pragma once

#include "component.hpp"
#include "entity.cpp"
#include "entity-component.cpp"
#include "ifb-component.hpp"
#include "ifb-types.hpp"
#include "physics-entity.cpp"
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

        const bool did_destroy = entity_destroy(id);
        return(did_destroy);
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

    IFB_ENGINE_API bool
    eng_entity_lookup_position(
        const entity_id id,
        position_3d&    pos) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_position);        
           
        if (result) {
            cmpnt_lookup_position(e.index_sparse, pos);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_velocity(
        const entity_id id,
        velocity_3d&    vel) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_velocity);        
           
        if (result) {
            cmpnt_lookup_velocity(e.index_sparse, vel);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_acceleration(
        const entity_id  id,
        acceleration_3d& acc) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_acceleration);        
           
        if (result) {
            cmpnt_lookup_acceleration(e.index_sparse, acc);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_term_velocity(
        const entity_id   id,
        term_velocity_3d& tv) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_term_velocity);        
           
        if (result) {
            cmpnt_lookup_term_velocity(e.index_sparse, tv);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_inv_mass(
        const entity_id id,
        f32&            im) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_term_velocity);        
           
        if (result) {
            inv_mass im_;
            cmpnt_lookup_inv_mass(e.index_sparse, im_);
            im = im_.normal_val;
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_mass(
        const entity_id id,
        f32&            mass) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_inv_mass);        
           
        if (result) {
            inv_mass im;
            cmpnt_lookup_inv_mass(e.index_sparse, im);
            mass = 1.0f / im.normal_val;
        }

        return(result);
    }
    
    IFB_ENGINE_API bool
    eng_entity_lookup_drag(
        const entity_id id,
        f32&            d) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_drag);        
           
        if (result) {
            drag d_;
            cmpnt_lookup_drag(e.index_sparse, d_);
            d = d_.normal_val;
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_quad(
        const entity_id id,
        quad&           q) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_quad);        
           
        if (result) {
            drag d_;
            cmpnt_lookup_quad(e.index_sparse, q);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_color(
        const entity_id id,
        color_rgba_u32& color) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_color);        
           
        if (result) {
            cmpnt_lookup_color(e.index_sparse, color);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_entity_lookup_spring(
        const entity_id id,
        spring&         spr) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_spring);        
           
        if (result) {
            cmpnt_lookup_spring(e.index_sparse, spr);
        }

        return(result);

    }

    IFB_ENGINE_API const bool
    eng_entity_remove_components(
        const entity_id      id,
        const component_type type) {
    
        assert(id != ENTITY_ID_INVALID); 

        const bool did_remove = entity_component_remove(id, type);
        return(did_remove);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_position(
        const entity_id    id,
        const position_3d& pos) {
    
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_position); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position(e.index_sparse, pos);   
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_velocity(
        const entity_id    id,
        const velocity_3d& vel) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_position); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_velocity(e.index_sparse,vel);   
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_acceleration(
        const entity_id        id,
        const acceleration_3d& acc) {
        
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_acceleration); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_acceleration(e.index_sparse, acc);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_term_velocity(
        const entity_id         id,
        const term_velocity_3d& tv) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_term_velocity); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_term_velocity(e.index_sparse, tv);
        }

        return(can_update);
    }
    
    IFB_ENGINE_API const bool
    eng_entity_update_inv_mass(
        const entity_id id,
        const f32       inv_mass) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_inv_mass); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct inv_mass im = {inv_mass};
            cmpnt_update_inv_mass(e.index_sparse, im);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_inv_mass); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct inv_mass im = {1 / mass};
            cmpnt_update_inv_mass(e.index_sparse, im);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_drag(
        const entity_id id,
        const f32       drag) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_drag); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct drag d = { drag };
            cmpnt_update_drag(e.index_sparse, d);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_quad(
        const entity_id       id,
        const quad_archetype& quad_atype) {
    
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_quad); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {

            quad q;
            q.height = quad_atype.dimensions.height;
            q.width  = quad_atype.dimensions.width;

            cmpnt_update_quad     (e.index_sparse, q);
            cmpnt_update_color    (e.index_sparse, quad_atype.color);
            cmpnt_update_position (e.index_sparse, quad_atype.position);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_color(
        const entity_id       id,
        const color_rgba_u32& color) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_quad); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_color(e.index_sparse, color);
        }

        return(can_update);
    }

    IFB_ENGINE_API const bool
    eng_entity_update_spring(
        const entity_id id,
        const spring&   spr) {

        assert(id != ENTITY_ID_INVALID); 

        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_spring); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_spring(e.index_sparse, spr);
        }

        return(can_update);
    }
    
    IFB_ENGINE_API const bool
    eng_entity_render(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e,
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

    IFB_ENGINE_API const bool
    eng_entity_add_force(
        const entity_id id,
        const vec3&     force) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e,
           cmpnt_type_e_position     |
           cmpnt_type_e_velocity     |
           cmpnt_type_e_acceleration |
           cmpnt_type_e_inv_mass     |
           cmpnt_type_e_drag
        ); 
        const bool can_add_force = (does_exist && has_component); 

        if (can_add_force) {
            physics_entity_add_force(id, force);
        }

        return(can_add_force);
        
    }
};
