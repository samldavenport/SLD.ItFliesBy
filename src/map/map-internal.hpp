#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "map.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // MAP MANAGER 
    //--------------------------------------------------------------------

    struct map_mngr {
        map_memory*     memory;
        u32             tiles_per_map;
        f32             tile_unit_size;
    } static * _map_mngr;

    //--------------------------------------------------------------------
    // MAP MEMORY
    //--------------------------------------------------------------------
    
    IFB_INTERNAL map_memory*       map_memory_create      (void);
    IFB_INTERNAL void              map_memory_init        (map_memory* map_mem, const memory& res);  
    IFB_INTERNAL map_memory_block* map_memory_alloc       (map_memory* map_mem);  
    IFB_INTERNAL void              map_memory_free        (map_memory* map_mem, map_memory_block* block);  
    IFB_INTERNAL void              map_memory_block_reset (map_memory_block* block);
    IFB_INTERNAL void*             map_memory_block_push  (map_memory_block* block, const u32 size);

    struct map_memory_block {
        map_memory_block* next;
        map_memory_block* prev;
        stack             stack;
    };

    struct map_memory : memory {
        map_memory_block* free;
        map_memory_block* used;
        u32               block_count; 
        u32               block_size; 
    };
};

#endif //MAP_INTERNAL_HPP
