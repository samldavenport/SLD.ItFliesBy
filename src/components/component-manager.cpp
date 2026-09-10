#pragma once

#include "component.hpp"
#include "eng-internal.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE cmpnt_table_position*      create_tbl_position      (void);
    IFB_INLINE cmpnt_table_color*         create_tbl_color         (void);
    IFB_INLINE cmpnt_table_quad*          create_tbl_quad          (void);
    IFB_INLINE cmpnt_table_rigid_body*    create_tbl_rigid_body    (void);
    IFB_INLINE cmpnt_table_velocity*      create_tbl_velocity      (void);
    IFB_INLINE cmpnt_table_acceleration*  create_tbl_acceleration  (void);
    IFB_INLINE cmpnt_table_inv_mass*      create_tbl_inv_mass      (void);
    IFB_INLINE cmpnt_table_drag*          create_tbl_drag          (void);
    IFB_INLINE cmpnt_table_term_velocity* create_tbl_term_velocity (void);
    IFB_INLINE cmpnt_table_spring*        create_tbl_spring        (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr*
    cmpnt_mngr_create(
        void) {

        _cmpnt_mngr = global_alloc<cmpnt_mngr>();
        assert(_cmpnt_mngr);

        return(_cmpnt_mngr);
    }

    IFB_INTERNAL void
    cmpnt_mngr_startup(
        reservation* res) {

        assert(res);

        const auto& cfg = config_instance();

        // calculate capacity
        _cmpnt_mngr->capacity = (cfg.entity_capacity / cfg.sparse_set_max_load_p100);
        assert(_cmpnt_mngr->capacity != 0);

        // commit memory to a stack
        _cmpnt_mngr->mem = reservation_push_stack_all(res);
        assert(_cmpnt_mngr->mem);

        // create tables
        _cmpnt_mngr->tbl.position.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.color.stack_init         (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.quad.stack_init          (_cmpnt_mngr->mem); 
        _cmpnt_mngr->tbl.rigid_body.stack_init    (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.velocity.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.acceleration.stack_init  (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.inv_mass.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.drag.stack_init          (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.term_velocity.stack_init (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.spring.stack_init        (_cmpnt_mngr->mem);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE cmpnt_table_position*
    create_tbl_position(
        void) {

        stack* s = _cmpnt_mngr->mem;

        auto tbl   = s->push_struct<cmpnt_table_position>();
        auto col_x = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(
            tbl   != NULL &&
            col_x != NULL &&
            col_y != NULL &&
            col_z != NULL
        );
    
        tbl->x = col_x;
        tbl->y = col_y;
        tbl->z = col_z;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_color*
    create_tbl_color(
        void) {

        stack* s = _cmpnt_mngr->mem;
        
        auto tbl     = s->push_struct<cmpnt_table_color>();
        auto col_hex = s->push_struct<u32>(_cmpnt_mngr->capacity);

        assert(
            tbl     != NULL &&
            col_hex != NULL
        );

        tbl->rgba_hex = col_hex;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_quad*
    create_tbl_quad(
        void) {

        auto s = _cmpnt_mngr->mem;

        auto tbl    = s->push_struct<cmpnt_table_quad> ();
        auto width  = s->push_struct<f32>              (_cmpnt_mngr->capacity);
        auto height = s->push_struct<f32>              (_cmpnt_mngr->capacity);

        assert(
            tbl    != NULL &&
            width  != NULL &&
            height != NULL            
        );

        tbl->width  = width; 
        tbl->height = height; 

        return(tbl);
    }

    IFB_INLINE cmpnt_table_rigid_body*
    create_tbl_rigid_body(
        void) {

        auto mem          = _cmpnt_mngr->mem;
        auto tbl          = mem->push_struct<cmpnt_table_rigid_body>();
        auto col_origin_x = mem->push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_origin_y = mem->push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_origin_z = mem->push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_width    = mem->push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_height   = mem->push_struct<f32>(_cmpnt_mngr->capacity); 

        assert(
            tbl          != NULL &&
            col_origin_x != NULL &&
            col_origin_y != NULL &&
            col_origin_z != NULL &&
            col_width    != NULL &&
            col_height   != NULL            
        );

        tbl->origin_x = col_origin_x;
        tbl->origin_y = col_origin_y;
        tbl->origin_z = col_origin_z;
        tbl->width    = col_width;
        tbl->height   = col_height;        

        return(tbl);
    }

    IFB_INLINE cmpnt_table_velocity*
    create_tbl_velocity(
        void) {

        auto s = _cmpnt_mngr->mem;

        auto tbl   = s->push_struct<cmpnt_table_velocity>();
        auto col_x = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(
            tbl   != NULL &&
            col_x != NULL &&
            col_y != NULL &&
            col_z != NULL
        );

        tbl->x = col_x;
        tbl->y = col_y;
        tbl->z = col_z;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_acceleration*
    create_tbl_acceleration(
        void) {
       
        auto s = _cmpnt_mngr->mem;

        auto tbl   = s->push_struct<cmpnt_table_acceleration>();
        auto col_x = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(
            tbl   != NULL &&
            col_x != NULL &&
            col_y != NULL &&
            col_z != NULL
        );

        tbl->x = col_x;
        tbl->y = col_y;
        tbl->z = col_z;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_inv_mass*
    create_tbl_inv_mass(
        void) {

        auto s = _cmpnt_mngr->mem;

        auto tbl     = s->push_struct<cmpnt_table_inv_mass>();
        auto col_val = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(tbl);
        assert(col_val);

        tbl->normal_val = col_val;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_drag*
    create_tbl_drag(
        void) {

        auto s = _cmpnt_mngr->mem;

        auto tbl     = s->push_struct<cmpnt_table_drag>();
        auto col_val = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(tbl);
        assert(col_val);

        tbl->normal_val = col_val;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_term_velocity*
    create_tbl_term_velocity(
        void) {
       
        auto s = _cmpnt_mngr->mem;

        auto tbl   = s->push_struct<cmpnt_table_term_velocity>();
        auto col_x = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = s->push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = s->push_struct<f32>(_cmpnt_mngr->capacity);
    
        assert(
            tbl   != NULL &&
            col_x != NULL &&
            col_y != NULL &&
            col_z != NULL
        );

        tbl->x = col_x;
        tbl->y = col_y;
        tbl->z = col_z;

        return(tbl);
    }

    IFB_INLINE cmpnt_table_spring*
    create_tbl_spring(
        void) {

        auto s = _cmpnt_mngr->mem;

        auto tbl    = s->push_struct<cmpnt_table_spring>();
        auto anchor = s->push_struct<entity_id> (_cmpnt_mngr->capacity);
        auto stiff  = s->push_struct<f32>       (_cmpnt_mngr->capacity);
        auto damp   = s->push_struct<f32>       (_cmpnt_mngr->capacity);
        auto rest   = s->push_struct<f32>       (_cmpnt_mngr->capacity);

        assert(tbl    != NULL);
        assert(anchor != NULL);
        assert(stiff  != NULL);
        assert(damp   != NULL);
        assert(rest   != NULL);

        tbl->anchor      = anchor;
        tbl->stiffness   = stiff;
        tbl->damping     = damp;
        tbl->rest_length = rest;

        return(tbl);
    }
};
