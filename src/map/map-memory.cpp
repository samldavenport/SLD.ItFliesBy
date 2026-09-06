#pragma once

#include "eng-internal.hpp"
#include "ifb-collections.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "map-internal.hpp"
#include "map.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL map_memory*
    map_memory_create(void) {

        auto memory = global_alloc<map_memory>();
        assert(memory != NULL);

        zero_memory((void*)memory, sizeof(map_memory));

        return(memory);
    }
    
    IFB_INTERNAL void
    map_memory_init(
        map_memory*   map_mem,
        const memory& res) {

        assert(map_mem != NULL);

        // commit memory
        map_mem->size = res.size;
        map_mem->ptr  = pfm_memory_commit(res.ptr, 0, res.size);
        assert(map_mem->size != 0);
        assert(map_mem->ptr  != 0);
    
        const auto& cfg = config_instance();  
    
        // determine block size and count
        map_mem->block_size  = cfg.map_block_size;
        map_mem->block_count = map_mem->size / map_mem->block_size;
        assert(map_mem->block_size);
        assert(map_mem->block_count);

        // set used block list to null
        map_mem->used = NULL;

        // initialize free block list
        memory stack_memory = {0};
        map_mem->free = (map_memory_block*)map_mem->ptr;
        map_memory_block* curr = NULL;
        map_memory_block* prev = map_mem->free;
        stack_memory.address = (addr)prev          + sizeof(map_memory_block);
        stack_memory.size    = map_mem->block_size - sizeof(map_memory_block); 
        prev->stack.init(stack_memory);
        prev->prev = NULL;

        for (
            u32 block_index = 0;
            block_index < map_mem->block_count - 1;
            ++block_index
        ) {
            curr       = (map_memory_block*)((addr)prev + map_mem->block_size);
            curr->next = NULL;
            curr->prev = prev;

            stack_memory.address = (addr)curr + sizeof(map_memory_block);
            curr->stack.init(stack_memory);

            prev->next = curr;
            prev       = curr;
        }
    } 
    
    IFB_INTERNAL map_memory_block*
    map_memory_alloc(
        map_memory* map_mem) {

        assert(map_mem);

        // get the next free block
        map_memory_block* block = map_mem->free;
        if (block == NULL) return(NULL);

        // initialize the block
        map_mem->free = block->next; 
        block->prev   = NULL;
        block->next   = map_mem->used;
        block->stack.reset();

        // update the next used block
        map_memory_block* next_used = map_mem->used;
        next_used->prev             = block;

        // add the block to the used list
        map_mem->used = block;
    
        return(block);
    }  

    IFB_INTERNAL void
    map_memory_free(
        map_memory*       map_mem,
        map_memory_block* block) {

        assert(map_mem);
        assert(block);

        auto next_used = block->next;
        auto prev_used = block->prev;

        // remove the block from the used list
        if (next_used)              next_used->prev = prev_used;
        if (prev_used)              prev_used->next = next_used;
        if (block == map_mem->used) map_mem->used   = next_used;

        // add it to the free list
        auto next_free = map_mem->free;
        if (next_free) next_free->prev = block;
        block->next   = next_free;
        map_mem->free = block;
    } 

    IFB_INTERNAL void
    map_memory_block_reset(
        map_memory_block* block) {

        assert(block); 
        block->stack.reset();
    }
    
    IFB_INTERNAL void*
    map_memory_block_push(
        map_memory_block* block,
        const u32         size) {

        assert(block != NULL);   
        assert(size  != 0);

        void* ptr = block->stack.push(size);
        return(ptr); 
    }

};
