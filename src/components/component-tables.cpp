#pragma once

#include "component.hpp"
#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // LOOKUP METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    cmpnt_lookup_position(
        const u32    sparse_index,
        position_3d& pos) {

        _cmpnt_mngr->tbl.position.lookup(sparse_index, pos);
    }
    
    IFB_INTERNAL void
    cmpnt_lookup_color(
        const u32       sparse_index,
        color_rgba_u32& clr) {

        _cmpnt_mngr->tbl.color.lookup(sparse_index, clr);
    }

    IFB_INTERNAL void
    cmpnt_lookup_quad          (
        const u32 sparse_index,
        dimensions_2d& dims) {
  
        _cmpnt_mngr->tbl.quad.lookup(sparse_index, q);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_rigid_body    (
        const u32 sparse_index,
        rigid_body&       rb) {
  
        _cmpnt_mngr->tbl.rigid_body.lookup(sparse_index, rb);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_velocity      (
        const u32 sparse_index,
        velocity_3d&      vel) {
  
        _cmpnt_mngr->tbl.velocity.lookup(sparse_index, vel);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_acceleration  (
        const u32 sparse_index,
        acceleration_3d&  acc) {
  
        _cmpnt_mngr->tbl.acceleration.lookup(sparse_index, acc);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_inv_mass      (
        const u32 sparse_index,
        inv_mass&         im) {
  
        _cmpnt_mngr->tbl.inv_mass.lookup(sparse_index, im);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_drag          (
        const u32 sparse_index,
        drag&             d) {
  
        _cmpnt_mngr->tbl.drag.lookup(sparse_index, d);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_term_velocity (
        const u32 sparse_index,
        term_velocity_3d& tv) {
  
        _cmpnt_mngr->tbl.term_velocity.lookup(sparse_index, tv);     
    }
   
    IFB_INTERNAL void
    cmpnt_lookup_spring        (
        const u32 sparse_index,
        spring&           s) {
  
        _cmpnt_mngr->tbl.spring.lookup(sparse_index, s);     
    }

    //--------------------------------------------------------------------
    // UPDATE METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void
    cmpnt_update_position(
        const u32          sparse_index,
        const position_3d& pos){
   
        _cmpnt_mngr->tbl.position.update(sparse_index, pos);
    }       
     
    IFB_INTERNAL void
    cmpnt_update_color(
        const u32             sparse_index,
        const color_rgba_u32& clr){
    
        _cmpnt_mngr->tbl.color.update(sparse_index, clr);
    } 
     
    IFB_INTERNAL void
    cmpnt_update_quad(
        const u32   sparse_index,
        const quad& q) {
    
        _cmpnt_mngr->tbl.quad.update(sparse_index, q);
    }

    IFB_INTERNAL void
    cmpnt_update_rigid_body(
        const u32         sparse_index,
        const rigid_body& rb){
        
        _cmpnt_mngr->tbl.rigid_body.update(sparse_index, rb);
    }   
     
    IFB_INTERNAL void
    cmpnt_update_velocity(
        const u32          sparse_index,
        const velocity_3d& vel){
    
        _cmpnt_mngr->tbl.velocity.update(sparse_index, vel);
    } 
     
    IFB_INTERNAL void
    cmpnt_update_acceleration(
        const u32              sparse_index,
        const acceleration_3d& acc){
    
        _cmpnt_mngr->tbl.acceleration.update(sparse_index, acc);
    } 
     
    IFB_INTERNAL void
    cmpnt_update_inv_mass(
        const u32       sparse_index,
        const inv_mass& im){
    
        _cmpnt_mngr->tbl.inv_mass.update(sparse_index, im);
    } 
     
    IFB_INTERNAL void
    cmpnt_update_drag(
        const u32   sparse_index,
        const drag& d){
       
        _cmpnt_mngr->tbl.drag.update(sparse_index,d);
    }
     
    IFB_INTERNAL void
    cmpnt_update_term_velocity(
        const u32               sparse_index,
        const term_velocity_3d& tv){
    
        _cmpnt_mngr->tbl.term_velocity.update(sparse_index, tv);
    }
   
    IFB_INTERNAL void
    cmpnt_update_spring(
        const u32     sparse_index,
        const spring& s){
    
        _cmpnt_mngr->tbl.spring.update(sparse_index, s);
    }
};
