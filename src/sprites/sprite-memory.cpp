#include "ifb-collections.hpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"
#include "eng-internal.hpp"
#include "memory.hpp"

namespace ifb {

    struct sprite_memory_block {
        stack* stack;
        u32    index;
    };
    
    struct sprite_memory {
        reservation*         res;
        b8                   block_is_allocated [config::sprite_mem_block_capacity];
        sprite_memory_block  blocks             [config::sprite_mem_block_capacity];
    };
    
    IFB_INTERNAL sprite_memory*
    sprite_memory_create(
        void) {

        auto mem = (sprite_memory*)global_alloc(sizeof(sprite_memory));
        assert(mem);

        return(mem);
    }

    IFB_INTERNAL void
    sprite_memory_init(
        sprite_memory* mem,
        reservation*   res) {

        assert(mem);
        assert(res);
        
        const u32 res_capacity = reservation_get_capacity(res);
        const u32 block_size   = res_capacity / config::sprite_mem_block_capacity;
    
        mem->res = res;
       
        for (
            u32 block_index = 0;
                block_index < config::sprite_mem_block_capacity;
              ++block_index) {

            mem->block_is_allocated [block_index] = false;

            auto& block = mem->blocks[block_index];
           
            memory stack_mem;
            stack_mem.size = block_size;
            stack_mem.ptr  = reservation_push_bytes(res, block_size);
            block.stack    = stack_memory_create(stack_mem);
            block.index    = block_index;
            assert(block.stack);
        }
    }
    
    IFB_INTERNAL sprite_memory_block*
    sprite_memory_alloc(
        sprite_memory* mem) {
       
        assert(mem);

        sprite_memory_block* block = NULL;
        for (
            u32 block_index = 0;
                block_index < config::sprite_mem_block_capacity;
              ++block_index) {

            if (!mem->block_is_allocated[block_index]) {

                block = &mem->blocks[block_index];
                stack_reset(block->stack);
            
                mem->block_is_allocated[block_index] = true;
                break;
            }
        }
    
        return(block);
    }

    IFB_INTERNAL void
    sprite_memory_free(
        sprite_memory*       mem,
        sprite_memory_block* block) {

        assert(mem);
        assert(block);
        assert(block->index < config::sprite_mem_block_capacity);

        mem->block_is_allocated[block->index] = false;
    }
    
    IFB_INTERNAL void*
    sprite_memory_block_push(
        sprite_memory_block* block,
        const u32            size) {

        assert(block);
        assert(size != 0);

        void* mem = stack_push(block->stack, size);
        return(mem);    
    }
    
    IFB_INTERNAL void
    sprite_memory_reset(
        sprite_memory_block* block) {

        assert(block);

        stack_reset(block->stack);
    }
};
