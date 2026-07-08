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
    quad_mngr_startup(
        memory& mem) {

        const auto& cfg = config_instance();

        // initialize the stack
        memory stack_mem;
        stack_mem.ptr  = pfm_memory_commit(mem.ptr, 0, mem.size);
        stack_mem.size = mem.size;
        _quad_mngr->stack_mem.init(stack_mem);
        _quad_mngr->capacity = cfg.quad_capacity;

        // allocate list memory
        auto list_all        = _quad_mngr->stack_mem.push_struct<quad_list>();
        auto list_to_render  = _quad_mngr->stack_mem.push_struct<quad_list>();
        auto list_to_update  = _quad_mngr->stack_mem.push_struct<quad_list>();
        auto quads_all       = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        auto quads_to_render = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        auto quads_to_update = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        assert(
            _quad_mngr->capacity != 0    &&            
            list_all             != NULL &&
            list_to_render       != NULL &&
            list_to_update       != NULL &&
            quads_all            != NULL &&
            quads_to_render      != NULL &&
            quads_to_update      != NULL            
        );

        // initialize lists
        list_all->array       = quads_all;
        list_all->count       = cfg.quad_capacity; 
        list_to_render->array = quads_to_render;
        list_to_render->count = 0;
        list_to_update->array = quads_to_update;
        list_to_update->count = 0; 
    }
};