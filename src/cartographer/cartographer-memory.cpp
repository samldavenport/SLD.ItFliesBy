#pragma once

#include "cartographer-internal.cpp"
#include "cartographer.hpp"
#include "ifb-platform.hpp"
#include "ifb-types.hpp"
#include "eng-internal.hpp"
#include <cassert>

namespace ifb {

    inline cartographer_memory_block* cartographer_memory_block_alloc (void);
    inline void                       cartographer_memory_block_free  (cartographer_memory_block* block);
    
    IFB_INTERNAL void
    cartographer_memory_init(
        memory& reserved_memory) {

        assert(reserved_memory.ptr  != NULL);
        assert(reserved_memory.size != 0);
        assert(_cartographer        != NULL);

        auto alctr = _cartographer->block_alctr; 
        assert(alctr);

        const u32 capacity = reserved_memory.size / sizeof(cartographer_memory_block);

        alctr->committed_memory.size = reserved_memory.size;  
        alctr->committed_memory.ptr  = pfm_memory_commit(reserved_memory.ptr, 0, reserved_memory.size);
        alctr->block_array           = (cartographer_memory_block*)alctr->committed_memory.ptr; 
        alctr->indexes_free          = global_alloc<u32>(capacity); 
        alctr->indexes_used          = global_alloc<u32>(capacity); 
        assert(alctr->committed_memory.ptr);
        assert(alctr->indexes_free);
        assert(alctr->indexes_used);

        for (
            u32 i = 0;
            i < capacity;
            ++i) {

            alctr->block_array[i].data = {0};
            alctr->block_array[i].header.block_index = i;
            alctr->indexes_free[i] = i;
        }
    }

    IFB_INTERNAL map*
    cartographer_memory_alloc_map(
        void) {

        auto block = cartographer_memory_block_alloc();
        if (!block) return(NULL);
       
        block->header.block_type  = cartographer_memory_block_type_e_map;
        block->data.map.next      = NULL;
        block->data.map.prev      = NULL;
        block->data.map.root_tile = NULL;
        block->data.map.tile_list = NULL;
        
        return(&block->data.map);
    }  

    IFB_INTERNAL tile*
    cartographer_memory_alloc_tile(
        void) {

        auto block = cartographer_memory_block_alloc();
        if (!block) return(NULL);
       
        block->header.block_type    = cartographer_memory_block_type_e_tile;
        block->data.tile.map        = NULL;
        block->data.tile.tile_north = NULL;
        block->data.tile.tile_south = NULL;
        block->data.tile.tile_east  = NULL;
        block->data.tile.tile_west  = NULL;
        block->data.tile.wall_north = NULL;
        block->data.tile.wall_south = NULL;
        block->data.tile.wall_east  = NULL;
        block->data.tile.wall_west  = NULL;

        return(&block->data.tile);
    }  
    
    IFB_INTERNAL wall*
    cartographer_memory_alloc_wall(
        void) {

        auto block = cartographer_memory_block_alloc();
        if (!block) return(NULL);
       
        block->header.block_type    = cartographer_memory_block_type_e_wall;
        block->data.wall.tile_north = NULL;
        block->data.wall.tile_south = NULL;
        block->data.wall.dir        = wall_direction_e_north_south;

        return(&block->data.wall);
    }

    IFB_INTERNAL atlas*
    cartorapher_memory_alloc_atlas(
        void) {

        auto block = cartographer_memory_block_alloc();
        if (!block) return(NULL);
       
        block->header.block_type          = cartographer_memory_block_type_e_atlas;
        block->data.atlas.keys            = NULL;
        block->data.atlas.image_data_ptr  = NULL;
        block->data.atlas.image_data_size = 0;
        block->data.atlas.image_count     = 0;

        return(&block->data.atlas);
    } 

    IFB_INTERNAL atlas_key* 
    cartographer_memory_alloc_atlas_key(
        void) {

        auto block = cartographer_memory_block_alloc();
        if (!block) return(NULL);
       
        block->header.block_type          = cartographer_memory_block_type_e_atlas_key;
        block->data.atlas_key.atlas       = NULL;
        block->data.atlas_key.index       = INVALID_INDEX;
        block->data.atlas_key.unit_x      = INVALID_INDEX;
        block->data.atlas_key.unit_y      = INVALID_INDEX;
        block->data.atlas_key.unit_length = INVALID_INDEX;
        block->data.atlas_key.unit_width  = INVALID_INDEX;

        return(&block->data.atlas_key);
    }  

    inline cartographer_memory_block*
    cartographer_memory_block_alloc(
        void) {

        assert(_cartographer);

        auto alctr = _cartographer->block_alctr;
        assert(alctr                        != NULL);
        assert(alctr->committed_memory.ptr  != NULL);
        assert(alctr->committed_memory.size != 0);

        auto block           = alctr->list_free;
        auto block_next_used = alctr->list_used;
        if (!block) return(NULL);

        assert(block->header.prev == NULL);
        
        if (block_next_used) {
            block_next_used->header.prev = block;
        } 

        block->header.next = block_next_used; 
        alctr->list_used   = block;

        return(block);
    }

    inline void
    cartographer_memory_return_used_block(
        cartographer_memory_block* block) {

        assert(block != NULL);
        assert(_cartographer);

        auto alctr = _cartographer->block_alctr;
        assert(alctr                        != NULL);
        assert(alctr->committed_memory.ptr  != NULL);
        assert(alctr->committed_memory.size != 0);
       
        auto used_next = block->header.next;
        auto used_prev = block->header.prev;

        if (alctr->list_used == block) {
            assert(used_prev == NULL);
            alctr->list_used = used_next;
        }
        else {
            assert(used_prev != NULL);
            used_prev->header.next = used_next;
        }

        if (used_next) {
            used_next->header.prev = used_prev;
        }

        auto free_first = alctr->list_free;
        block->header.prev = NULL;
        block->header.next = free_first; 

        if (free_first) {
            free_first->header.prev = block;
        }

        alctr->list_free = block;
    }
    
    IFB_INTERNAL void
    cartographer_memory_free_map(
        map* m) {

        assert(m);
        assert(_cartographer);
        assert(_cartographer->block_alctr);
    
        const addr a = (addr)m;
        const addr b = b - sizeof(cartographer_memory_block::header);
       
        auto block = (cartographer_memory_block*)b;
        assert(&block->data.map == m);

        cartographer_memory_block_free(block);
    }

    IFB_INTERNAL void
    cartographer_memory_free_tile(
        tile* t) {

        assert(t);
        assert(_cartographer);
        assert(_cartographer->block_alctr);
    
        const addr a = (addr)t;
        const addr b = b - sizeof(cartographer_memory_block::header);
       
        auto block = (cartographer_memory_block*)b;
        assert(&block->data.tile == t);

        cartographer_memory_block_free(block);
    } 

    IFB_INTERNAL void
    cartographer_memory_free_wall(
        wall* w) {

        assert(w);
        assert(_cartographer);
        assert(_cartographer->block_alctr);
    
        const addr a = (addr)w;
        const addr b = b - sizeof(cartographer_memory_block::header);
       
        auto block = (cartographer_memory_block*)b;
        assert(&block->data.wall == w);

        cartographer_memory_block_free(block);

    }

    IFB_INTERNAL void
    cartographer_memory_free_atlas(
        atlas* at) {

        assert(at);
        assert(_cartographer);
        assert(_cartographer->block_alctr);
    
        const addr a = (addr)at;
        const addr b = b - sizeof(cartographer_memory_block::header);
       
        auto block = (cartographer_memory_block*)b;
        assert(&block->data.atlas == at);

        cartographer_memory_block_free(block);
    }

    IFB_INTERNAL void
    cartographer_memory_free_atlas_key(
        atlas_key* ak) {

        assert(ak);
        assert(_cartographer);
        assert(_cartographer->block_alctr);
    
        const addr a = (addr)ak;
        const addr b = b - sizeof(cartographer_memory_block::header);
       
        auto block = (cartographer_memory_block*)b;
        assert(&block->data.atlas_key == ak);

        cartographer_memory_block_free(block);
    }
};
