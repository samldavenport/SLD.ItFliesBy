#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_drag_validate(
        void) {

        assert(_cmpnt_mngr                  != NULL);     
        assert(_cmpnt_mngr->capacity        != 0);     
        assert(_cmpnt_mngr->tables.drag != NULL);     
    }
    
    IFB_INTERNAL void
    cmpnt_table_drag_update(
        const u32       sparse_index,
        const drag& d) {

        cmpnt_table_drag_validate();  
        
        assert(sparse_index < _cmpnt_mngr->capacity);
        
        auto tbl = _cmpnt_mngr->tables.drag;
        
        tbl->normal_val[sparse_index] = (d.normal_val >= 0.0f && d.normal_val <= 1.0f)
            ? d.normal_val
            : 1.0f;
    }
    
    IFB_INTERNAL void
    cmpnt_table_drag_lookup(
        const u32 sparse_index,
        drag& d) {

        cmpnt_table_drag_validate();  
         
        assert(sparse_index < _cmpnt_mngr->capacity);
   
        auto& tbl = _cmpnt_mngr->tables.drag;

        d.normal_val = tbl->normal_val[sparse_index];
    }
};
