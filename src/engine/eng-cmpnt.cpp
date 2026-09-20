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

    //--------------------------------------------------------------------
    // LOOKUP METHODS 
    //--------------------------------------------------------------------

    IFB_ENGINE_API bool
    eng_cmpnt_lookup_position(
        const entity_id id,
        cmpnt_position&    pos) {

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
    eng_cmpnt_lookup_velocity(
        const entity_id id,
        cmpnt_velocity&    vel) {

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
    eng_cmpnt_lookup_acceleration(
        const entity_id  id,
        cmpnt_acceleration& acc) {

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
    eng_cmpnt_lookup_term_velocity(
        const entity_id   id,
        cmpnt_term_velocity& tv) {

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
    eng_cmpnt_lookup_inv_mass(
        const entity_id id,
        f32&            im) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_term_velocity);        
           
        if (result) {
            cmpnt_inv_mass im_;
            cmpnt_lookup_inv_mass(e.index_sparse, im_);
            im = im_.normal_val;
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_lookup_mass(
        const entity_id id,
        f32&            mass) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_inv_mass);        
           
        if (result) {
            cmpnt_inv_mass im;
            cmpnt_lookup_inv_mass(e.index_sparse, im);
            mass = 1.0f / im.normal_val;
        }

        return(result);
    }
    
    IFB_ENGINE_API bool
    eng_cmpnt_lookup_drag(
        const entity_id id,
        f32&            d) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_drag);        
           
        if (result) {
            cmpnt_drag d_;
            cmpnt_lookup_drag(e.index_sparse, d_);
            d = d_.normal_val;
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_lookup_quad(
        const entity_id id,
        cmpnt_quad&     q) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_quad);        
           
        if (result) {
            cmpnt_drag d_;
            cmpnt_lookup_quad(e.index_sparse, q);
        }

        return(result);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_lookup_color(
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
    eng_cmpnt_lookup_spring(
        const entity_id id,
        cmpnt_spring&   spr) {

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

    IFB_ENGINE_API bool
    eng_cmpnt_lookup_map_coords(
        const entity_id   id,
        cmpnt_map_coords& coords) {

        assert(id != ENTITY_ID_INVALID);

        bool result = true;

        entity e;
        result  &= entity_lookup_by_id  (e, id);
        result  &= entity_has_component (e, cmpnt_type_e_map_coords);        
           
        if (result) {
            cmpnt_lookup_map_coords(e.index_sparse, coords);
        }

        return(result);

    }
    
    IFB_ENGINE_API bool
    eng_entity_remove_components(
        const entity_id      id,
        const component_type type) {
    
        assert(id != ENTITY_ID_INVALID); 

        bool did_remove = entity_component_remove(id, type);
        return(did_remove);
    }

    //--------------------------------------------------------------------
    // LOOKUP METHODS 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool
    eng_cmpnt_update_position(
        const entity_id    id,
        const cmpnt_position& pos) {
    
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_position); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_position(e.index_sparse, pos);   
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_update_velocity(
        const entity_id    id,
        const cmpnt_velocity& vel) {

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

    IFB_ENGINE_API bool
    eng_cmpnt_update_acceleration(
        const entity_id        id,
        const cmpnt_acceleration& acc) {
        
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

    IFB_ENGINE_API bool
    eng_cmpnt_update_term_velocity(
        const entity_id         id,
        const cmpnt_term_velocity& tv) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_term_velocity); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_term_velocity(e.index_sparse, tv);
        }

        return(can_update);
    }
    
    IFB_ENGINE_API bool
    eng_cmpnt_update_inv_mass(
        const entity_id id,
        const f32       inv_mass) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_inv_mass); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct cmpnt_inv_mass im = {inv_mass};
            cmpnt_update_inv_mass(e.index_sparse, im);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_update_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_inv_mass); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct cmpnt_inv_mass im = {1 / mass};
            cmpnt_update_inv_mass(e.index_sparse, im);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_update_drag(
        const entity_id id,
        const f32       drag) {

        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_drag); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {

            struct cmpnt_drag d = { drag };
            cmpnt_update_drag(e.index_sparse, d);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_update_quad(
        const entity_id       id,
        const atype_quad& quad_atype) {
    
        assert(id != ENTITY_ID_INVALID); 
   
        entity e;
        bool does_exist    = entity_lookup_by_id  (e, id);
        bool has_component = entity_has_component (e, cmpnt_type_e_quad); 
        bool can_update    = (does_exist && has_component); 

        if (can_update) {

            cmpnt_quad q;
            q.height = quad_atype.quad.height;
            q.width  = quad_atype.quad.width;

            cmpnt_update_quad     (e.index_sparse, q);
            cmpnt_update_color    (e.index_sparse, quad_atype.color);
            cmpnt_update_position (e.index_sparse, quad_atype.position);
        }

        return(can_update);
    }

    IFB_ENGINE_API bool
    eng_cmpnt_update_color(
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

    IFB_ENGINE_API bool
    eng_cmpnt_update_spring(
        const entity_id     id,
        const cmpnt_spring& spr) {

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
    
    IFB_ENGINE_API bool
    eng_cmpnt_update_map_coords(
        const entity_id         id,
        const cmpnt_map_coords& coords) {

        assert(id != ENTITY_ID_INVALID); 

        entity e;
        const bool does_exist    = entity_lookup_by_id  (e, id);
        const bool has_component = entity_has_component (e, cmpnt_type_e_map_coords); 
        const bool can_update    = (does_exist && has_component); 

        if (can_update) {
            cmpnt_update_map_coords(e.index_sparse, coords);
        }

        return(can_update);
    }
    
    //--------------------------------------------------------------------
    // UPDATE METHODS 
    //--------------------------------------------------------------------

};
