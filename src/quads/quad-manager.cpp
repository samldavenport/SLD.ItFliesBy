#pragma once

#include "quad.hpp"
#include "eng-internal.hpp"

namespace ifb {


    IFB_INTERNAL quad_mngr*
    quad_mngr_create(
        void) {

        _quad_mngr = global_alloc<quad_mngr>();
        assert(_quad_mngr);

        return(_quad_mngr);
    }

    IFB_INTERNAL void
    quad_mngr_validate(
        void) {

        assert(_quad_mngr);
    }

    IFB_INTERNAL void
    quad_mngr_startup(
        memory& mem) {

        const auto& cfg = config_instance();

        // initialize the stack
        memory stack_mem;
        stack_mem.ptr  = pfm_memory_commit(mem.ptr, 0, mem.size);
        stack_mem.size = mem.size;
        _quad_mngr->stack_mem.init(stack_mem);

        // allocate list memory
        auto quads_all = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        assert(quads_all != NULL);

        // initialize list
        _quad_mngr->all.init(quads_all, cfg.quad_capacity);
    }

};