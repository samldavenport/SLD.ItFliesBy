#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_inv_mass_validate(
        void) {

        assert(_cmpnt_mngr                  != NULL);     
        assert(_cmpnt_mngr->capacity        != 0);     
        assert(_cmpnt_mngr->tables.inv_mass != NULL);     
    }

    IFB_INTERNAL void
    cmpnt_table_inv_mass_update(
        const u32       sparse_index,
        const inv_mass& inv_mass) {

        cmpnt_table_inv_mass_validate();  
        
        assert(sparse_index < _cmpnt_mngr->capacity);
        
        auto tbl = _cmpnt_mngr->tables.inv_mass;
        
        tbl->normal_val[sparse_index] = (inv_mass.normal_val >= 0.0f && inv_mass.normal_val <= 1.0f)
            ? inv_mass.normal_val
            : 1.0f;
    }
    
    IFB_INTERNAL void
    cmpnt_table_inv_mass_lookup(
        const u32 sparse_index,
        inv_mass& inv_mass) {

        cmpnt_table_inv_mass_validate();  
         
        assert(sparse_index < _cmpnt_mngr->capacity);
   
        auto& tbl = _cmpnt_mngr->tables.inv_mass;

        inv_mass.normal_val = tbl->normal_val[sparse_index];
    }
};
