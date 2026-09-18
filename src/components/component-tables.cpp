#pragma once

#include <cassert>
#include "component.hpp"
#include "ifb-types.hpp"
#include "component-internal.hpp"

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
        assert(tbl->array_rigid_body);
        assert(tbl->array_velocity);
        assert(tbl->array_acceleration);
        assert(tbl->array_inv_mass);
        assert(tbl->array_drag);
        assert(tbl->array_term_velocity);
        assert(tbl->array_spring);
        assert(tbl->array_map_coords);
    
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
    cmpnt_lookup_quad          (
        const u32 sparse_index,
        cmpnt_quad& q) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        q = tbl->array_quad[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_rigid_body    (
        const u32 sparse_index,
        cmpnt_rigid_body&       rb) {
  
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        rb = tbl->array_rigid_body[sparse_index];
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_velocity      (
        const u32 sparse_index,
        cmpnt_velocity&      vel) {
  
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
    cmpnt_update_rigid_body(
        const u32         sparse_index,
        const cmpnt_rigid_body& rb){
        
        const auto tbl = cmpnt_table_get_and_validate(sparse_index);
        tbl->array_rigid_body[sparse_index] = rb;
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
};
