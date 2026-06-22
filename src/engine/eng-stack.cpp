#pragma once

#include "eng-internal.hpp"

namespace ifb {

    IFB_ENG_INTERNAL eng_stack*
    eng_stack_init(
        const eng_mem_map* mem_map) {

        assert(
            mem_map             != NULL &&
            mem_map->stack.ptr  != NULL &&
            mem_map->stack.size != 0
        );

        zero_memory(mem_map->stack.ptr, mem_map->stack.size);

        eng_stack* stack = (eng_stack*)mem_map->stack.ptr;
        stack->position  = sizeof(eng_stack);
        stack->size      = mem_map->stack.size;

        return(stack);
    }

    IFB_ENG_INTERNAL byte*
    eng_stack_push_data(
        eng_stack* stack,
        const u32  size) {

        assert(stack);

        const u32 new_position = (stack->position + size);
        assert(new_position <= stack->size);

        byte* start = (byte*)stack;
        byte* data  = start + stack->position; 

        stack->position = new_position;

        return(data);
    }

    IFB_ENG_INTERNAL eng_context*
    eng_stack_push_context(
        eng_stack* stack) {

        assert(stack);

        auto ctx = (eng_context*)eng_stack_push_data(stack, sizeof(eng_context));
        assert(ctx);

        return(ctx);
    }
    
    IFB_ENG_INTERNAL eng_system_info*
    eng_stack_push_system_info(
        eng_stack* stack) {

        auto sys_info = (eng_system_info*)eng_stack_push_data(stack, sizeof(eng_system_info));
        assert(sys_info);
        return(sys_info);
    }

    IFB_ENG_INTERNAL file_manager*
    eng_stack_push_and_init_file_manager(
        eng_stack* stack,
        const u32  file_count_max) {

        assert(stack != NULL && file_count_max != 0);
        


        const u32 mem_req = file_manager_memory_requirement(file_count_max);
        void*     mem_ptr = (void*)eng_stack_push_data(stack, mem_req);
        assert(mem_ptr);

        auto file_mngr = file_manager_init(
            file_count_max,
            mem_req,
            mem_ptr
        );
        assert(file_mngr);
        return(file_mngr);
    }

    IFB_ENG_INTERNAL renderer_context*
    eng_stack_push_and_init_renderer(
        eng_stack* stack) {

        assert(stack != NULL);

        memory stack_mem;
        stack_mem.size  = renderer_context_memory_requirement ();
        stack_mem.bytes = eng_stack_push_data                 (stack, stack_mem.size);

        renderer_context* rndr = renderer_context_init_from_memory(stack_mem);
        assert(rndr);
        return(rndr);
    }

    IFB_ENG_INTERNAL eng_managers*
    eng_stack_push_and_init_managers(
        eng_stack* eng_stack) {

        assert(eng_stack != NULL);

        const auto& config = config_instance();

        auto mngrs = (eng_managers*)eng_stack_push_data(eng_stack, sizeof(eng_managers));
        assert(mngrs);

        memory stack_mem;

        // file manager
        stack_mem.size = file_manager_memory_requirement (config.file_count);
        stack_mem.ptr  = eng_stack_push_data             (eng_stack, stack_mem.size); 
        mngrs->file    = file_manager_init               (config.file_count, stack_mem.size, stack_mem.ptr);

        // entity manager
        stack_mem.size = entity_manager_memory_requirement ();
        stack_mem.ptr  = eng_stack_push_data               (eng_stack, stack_mem.size);
        mngrs->entity  = entity_manager_memory_init        (stack_mem); 

        assert(
            mngrs->file   != NULL &&            
            mngrs->entity != NULL
        );

        return(mngrs);
    }


    IFB_ENG_INTERNAL gui*
    eng_stack_push_and_init_gui(
        eng_stack* eng_stack) {

        assert(eng_stack != NULL);

        memory gui_stack_mem;
        gui_stack_mem.size = gui_memory_requirement();
        gui_stack_mem.ptr  = eng_stack_push_data(eng_stack, gui_stack_mem.size);

        gui* g = gui_memory_init(gui_stack_mem);
        assert(g);

        return(g);
    }


};