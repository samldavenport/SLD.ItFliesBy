#pragma once

#include "component.hpp"
#include "eng-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE cmpnt_table_position*     create_tbl_position     (void);
    IFB_INLINE cmpnt_table_color*        create_tbl_color        (void);
    IFB_INLINE cmpnt_table_quad*         create_tbl_quad         (void);
    IFB_INLINE cmpnt_table_rigid_body*   create_tbl_rigid_body   (void);
    IFB_INLINE cmpnt_table_velocity*     create_tbl_velocity     (void);
    IFB_INLINE cmpnt_table_acceleration* create_tbl_acceleration (void);

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
        memory& mem_res) {

        const auto& cfg = config_instance();

        // calculate capacity
        _cmpnt_mngr->capacity = (cfg.entity_capacity / cfg.sparse_set_max_load_p100);
        assert(_cmpnt_mngr->capacity != 0);

        // commit memory to a stack
        mem_res.ptr = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        assert(mem_res.ptr);
        _cmpnt_mngr->mem.init(mem_res);

        // create tables
        _cmpnt_mngr->tables.position     = create_tbl_position     ();
        _cmpnt_mngr->tables.color        = create_tbl_color        ();
        _cmpnt_mngr->tables.quad         = create_tbl_quad         (); 
        _cmpnt_mngr->tables.rigid_body   = create_tbl_rigid_body   ();
        _cmpnt_mngr->tables.velocity     = create_tbl_velocity     ();
        _cmpnt_mngr->tables.acceleration = create_tbl_acceleration ();
        assert(
            _cmpnt_mngr->tables.position     != NULL &&
            _cmpnt_mngr->tables.color        != NULL &&            
            _cmpnt_mngr->tables.quad         != NULL &&            
            _cmpnt_mngr->tables.rigid_body   != NULL &&
            _cmpnt_mngr->tables.velocity     != NULL &&
            _cmpnt_mngr->tables.acceleration != NULL
        );
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE cmpnt_table_position*
    create_tbl_position(
        void) {

        auto tbl   = _cmpnt_mngr->mem.push_struct<cmpnt_table_position>();
        auto col_x = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
    
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

        auto tbl     = _cmpnt_mngr->mem.push_struct<cmpnt_table_color>();
        auto col_hex = _cmpnt_mngr->mem.push_struct<u32>(_cmpnt_mngr->capacity);

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

        auto tbl    = _cmpnt_mngr->mem.push_struct<cmpnt_table_quad> ();
        auto width  = _cmpnt_mngr->mem.push_struct<f32>              (_cmpnt_mngr->capacity);
        auto height = _cmpnt_mngr->mem.push_struct<f32>              (_cmpnt_mngr->capacity);

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

        auto tbl          = _cmpnt_mngr->mem.push_struct<cmpnt_table_rigid_body>();
        auto col_origin_x = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_origin_y = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_origin_z = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_width    = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity); 
        auto col_height   = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity); 

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

        auto tbl   = _cmpnt_mngr->mem.push_struct<cmpnt_table_velocity>();
        auto col_x = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
    
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
        
        auto tbl   = _cmpnt_mngr->mem.push_struct<cmpnt_table_acceleration>();
        auto col_x = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_y = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
        auto col_z = _cmpnt_mngr->mem.push_struct<f32>(_cmpnt_mngr->capacity);
    
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
};