#pragma once

#include <cassert>
#include "ifb.hpp"
#include "eng-component.hpp"

namespace ifb {
   
    inline cmpnt_table*
    cmpnt_table_get_and_validate(
        const u32 sparse_index) {

        assert(_cmpnt_mngr);
        assert(_cmpnt_mngr->capacity > sparse_index);

        cmpnt_table* tbl = _cmpnt_mngr->cmpnt_tbl;
        assert(tbl);
        assert(tbl->array_position);
        assert(tbl->array_color);
        assert(tbl->array_quad);
        assert(tbl->array_velocity);
        assert(tbl->array_acceleration);
        assert(tbl->array_inv_mass);
        assert(tbl->array_drag);
        assert(tbl->array_term_velocity);
        assert(tbl->array_spring);
        assert(tbl->array_map_coords);
        assert(tbl->array_particle_emitter);
        assert(tbl->array_sprite);
        assert(tbl->array_bounds);
        assert(tbl->array_collider);

        return(tbl);
    }

    //--------------------------------------------------------------------
    // LOOKUP METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    cmpnt_lookup_position(
        const u32    sparse_index,
        cmpnt_position& pos) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        pos = tbl->array_position[sparse_index];
    }
    
    IFB_INTERNAL void
    cmpnt_lookup_color(
        const u32       sparse_index,
        color_rgba_u32& clr) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        clr = tbl->array_color[sparse_index]; 
    }

    IFB_INTERNAL void
    cmpnt_lookup_quad(
        const u32   sparse_index,
        cmpnt_quad& q) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        q = tbl->array_quad[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_velocity(
        const u32       sparse_index,
        cmpnt_velocity& vel) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        vel = tbl->array_velocity[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_acceleration(
        const u32 sparse_index,
        cmpnt_acceleration&  acc) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        acc = tbl->array_acceleration[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_inv_mass(
        const u32 sparse_index,
        cmpnt_inv_mass& im) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        im = tbl->array_inv_mass[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_drag(
        const u32   sparse_index,
        cmpnt_drag& d) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        d = tbl->array_drag[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_term_velocity (
        const u32 sparse_index,
        cmpnt_term_velocity& tv) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tv = tbl->array_term_velocity[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_spring        (
        const u32     sparse_index,
        cmpnt_spring& s) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        s = tbl->array_spring[sparse_index];
    }

    IFB_INTERNAL void
    cmpnt_lookup_map_coords(
        const u32   sparse_index,
        cmpnt_map_coords& mc) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        mc = tbl->array_map_coords[sparse_index];
    }

    IFB_INTERNAL void
    cmpnt_lookup_particle_emitter(
        const u32               sparse_index,
        cmpnt_particle_emitter& pe) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        pe = tbl->array_particle_emitter[sparse_index];
    }

    IFB_INTERNAL void
    cmpnt_lookup_sprite(
        const u32     sparse_index,
        cmpnt_sprite& sprite) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        sprite = tbl->array_sprite[sparse_index];
    }

    IFB_INTERNAL void
    cmpnt_lookup_bounds(
        const u32     sparse_index,
        cmpnt_bounds& bounds) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        bounds = tbl->array_bounds[sparse_index];
    }

    IFB_INTERNAL void
    cmpnt_lookup_collider(
        const u32       sparse_index,
        cmpnt_collider& collider) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        collider = tbl->array_collider[sparse_index];
    }

    //--------------------------------------------------------------------
    // UPDATE METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void
    cmpnt_update_position(
        const u32          sparse_index,
        const cmpnt_position& pos){
   
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_position[sparse_index] = pos;
    }       
     
    IFB_INTERNAL void
    cmpnt_update_color(
        const u32             sparse_index,
        const color_rgba_u32& clr){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_color[sparse_index] = clr;
    } 
     
    IFB_INTERNAL void
    cmpnt_update_quad(
        const u32   sparse_index,
        const cmpnt_quad& q) {
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_quad[sparse_index] = q;
    }
     
    IFB_INTERNAL void
    cmpnt_update_velocity(
        const u32          sparse_index,
        const cmpnt_velocity& vel){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_velocity[sparse_index] = vel;
    } 
     
    IFB_INTERNAL void
    cmpnt_update_acceleration(
        const u32              sparse_index,
        const cmpnt_acceleration& acc){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_acceleration[sparse_index] = acc;
    } 
     
    IFB_INTERNAL void
    cmpnt_update_inv_mass(
        const u32       sparse_index,
        const cmpnt_inv_mass& im){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_inv_mass[sparse_index] = im;
    } 
     
    IFB_INTERNAL void
    cmpnt_update_drag(
        const u32   sparse_index,
        const cmpnt_drag& d){
       
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_drag[sparse_index] = d;
    }
     
    IFB_INTERNAL void
    cmpnt_update_term_velocity(
        const u32               sparse_index,
        const cmpnt_term_velocity& tv){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_term_velocity[sparse_index] = tv;
    }
   
    IFB_INTERNAL void
    cmpnt_update_spring(
        const u32           sparse_index,
        const cmpnt_spring& s){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_spring[sparse_index] = s;
    }
    
    IFB_INTERNAL void
    cmpnt_update_map_coords(
        const u32         sparse_index,
        const cmpnt_map_coords& mc){
    
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_map_coords[sparse_index] = mc;
    }

    IFB_INTERNAL void
    cmpnt_update_particle_emitter(
        const u32                     sparse_index,
        const cmpnt_particle_emitter& pe) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_particle_emitter[sparse_index] = pe;
    }
    
    IFB_INTERNAL void
    cmpnt_update_sprite(
        const u32           sparse_index,
        const cmpnt_sprite& sprite) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_sprite[sparse_index] = sprite;
    }

    IFB_INTERNAL void
    cmpnt_update_bounds(
        const u32           sparse_index,
        const cmpnt_bounds& bounds) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_bounds[sparse_index] = bounds;
    }

    IFB_INTERNAL void
    cmpnt_update_collider(
        const u32             sparse_index,
        const cmpnt_collider& collider) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_collider[sparse_index] = collider;
    }

    //--------------------------------------------------------------------
    // INTERPOLATION
    //--------------------------------------------------------------------

    // called when a simulation step starts, before anything moves
    // a frame drawn after the step blends from these positions to the ones the step leaves
    IFB_INTERNAL void
    cmpnt_positions_begin_step(
        void) {

        IFB_PROFILE_FUNCTION();

        assert(_cmpnt_mngr);
        cmpnt_table* tbl = _cmpnt_mngr->cmpnt_tbl;
        assert(tbl);

        for (
            u32 index = 0;
                index < _cmpnt_mngr->capacity;
              ++index) {

            tbl->array_position_prev       [index] = tbl->array_position[index];
            tbl->array_position_prev_valid [index] = true;
        }
    }

    // 0 draws everything where the last step started it, 1 where the step left it
    IFB_INTERNAL void
    cmpnt_positions_set_render_alpha(
        const f32 alpha) {

        assert(_cmpnt_mngr);

        f32 a = alpha;
        if (a < 0.0f) a = 0.0f;
        if (a > 1.0f) a = 1.0f;
        _cmpnt_mngr->render_alpha = a;
    }

    // the entity is drawn exactly where it is until the next step starts,
    // for a new entity, whose slot's previous position was someone else's,
    // and for a teleport, which shouldn't be drawn sliding across the map
    IFB_INTERNAL void
    cmpnt_position_snap(
        const u32 sparse_index) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_position_prev_valid[sparse_index] = false;
    }

    // where to draw the entity in this frame, between its last two steps
    // only for drawing, everything else uses cmpnt_lookup_position
    IFB_INTERNAL void
    cmpnt_lookup_position_render(
        const u32       sparse_index,
        cmpnt_position& pos) {

        const auto tbl = cmpnt_table_get_and_validate(sparse_index);

        const cmpnt_position& curr = tbl->array_position[sparse_index];
        if (!tbl->array_position_prev_valid[sparse_index]) {
            pos = curr;
            return;
        }

        const cmpnt_position& prev = tbl->array_position_prev[sparse_index];
        const f32             t    = _cmpnt_mngr->render_alpha;
        pos.x = prev.x + ((curr.x - prev.x) * t);
        pos.y = prev.y + ((curr.y - prev.y) * t);
        pos.z = prev.z + ((curr.z - prev.z) * t);
    }
};
