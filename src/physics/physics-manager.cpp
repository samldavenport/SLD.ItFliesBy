#pragma once

#include "physics.hpp"
#include "eng-internal.hpp"

namespace ifb {
    
    IFB_INTERNAL physics_manager*
    physics_manager_create(
        void) {

        physics_manager* mngr = global_alloc<physics_manager>();
        physics_memory*  mem  = global_alloc<physics_memory>();

        assert(mngr != NULL);
        assert(mem  != NULL);
    
        mngr->memory     = mem;
        mngr->world_list = NULL;

        _phys_mngr = mngr;
        return(mngr);
    }

    IFB_INTERNAL void
    physics_manager_validate(
        void) {

        assert(_phys_mngr         != NULL);
        assert(_phys_mngr->memory != NULL);
    }

    IFB_INTERNAL void
    physics_manager_startup(
        memory& mem_res) {

        physics_manager_validate();
        auto phys_mem = _phys_mngr->memory; 

        assert(mem_res.address != 0);
        assert(mem_res.size    != 0);

        // commit stack memory
        memory mem_stack;
        mem_stack.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        mem_stack.size = mem_res.size;
        assert(mem_stack.ptr != NULL);

        // initialize stack
        phys_mem->stack.init(mem_stack);

        // calculate world block size and granularity
        const auto cfg = config_instance();
        const u32  block_granularity = cfg.physics_world_size;
        const u32  block_count       = cfg.physics_world_count;
        const u32  block_memory_size = (block_granularity * block_count);

        // intialize world block allocator
        memory block_mem;
        block_mem.ptr  = phys_mem->stack.push(block_memory_size);
        block_mem.size = block_memory_size;
        block_alctr_init(&phys_mem->world_allocator, block_mem, block_granularity);
    }

    IFB_INTERNAL void
    physics_manager_shutdown(
        void) {

        //TODO(SAM)
        
    }

};