#pragma once

#include "physics.hpp"
#include "eng-internal.hpp"

namespace ifb {
    
    IFB_INTERNAL physics_mngr*
    physics_mngr_create(
        void) {

        physics_mngr*   mngr = global_alloc<physics_mngr>();
        physics_memory* mem  = global_alloc<physics_memory>();

        assert(mngr != NULL);
        assert(mem  != NULL);
    
        mngr->memory     = mem;

        _phys_mngr = mngr;
        return(mngr);
    }

    IFB_INTERNAL void
    physics_mngr_validate(
        void) {

        assert(_phys_mngr                    != NULL);
        assert(_phys_mngr->memory            != NULL);
        assert(_phys_mngr->force_accumulator != NULL);
    }

    IFB_INTERNAL void
    physics_mngr_startup(
        memory& mem_res) {

        physics_mngr_validate();
        auto phys_mem = _phys_mngr->memory; 

        assert(mem_res.address != 0);
        assert(mem_res.size    != 0);

        // commit stack memory
        memory mem_stack;
        mem_stack.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        mem_stack.size = mem_res.size;
        assert(mem_stack.ptr != NULL);

        // initialize stack and remaining structures
        phys_mem->stack.init(mem_stack);
        _phys_mngr->force_accumulator = physics_accumulator_init(phys_mem->stack);
    }

    IFB_INTERNAL void
    physics_mngr_shutdown(
        void) {

        //TODO(SAM)
        
    }

};
