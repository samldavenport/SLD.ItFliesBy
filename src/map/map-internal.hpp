#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "ifb-types.hpp"
#include "map.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL TYPES 
    //--------------------------------------------------------------------
   
    struct map_memory_stack;
    struct map_memory_stack_list;
    struct map_memory;
    struct map_chunk;
    struct map;

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
   

    IFB_INTERNAL map_memory*       map_memory_create           (void);
    IFB_INTERNAL void              map_memory_init             (map_memory* map_mem, const memory& res);  
    IFB_INTERNAL map_memory_stack* map_memory_stack_alloc      (map_memory* map_mem);  
    IFB_INTERNAL void              map_memory_stack_free       (map_memory* map_mem, map_memory_stack* stack);  
    IFB_INTERNAL map*              map_memory_stack_push_map   (map_memory* map_mem, map_memory_stack* stack);
    IFB_INTERNAL map_chunk*        map_memory_stack_push_chunk (map_memory* map_mem, map_memory_stack* stack);

    struct map_memory_stack {
        map_memory_stack* next;
        map_memory_stack* prev;
        u32               pos; 
    };

    struct map_memory_stack_list {
        map_memory_stack* free;
        map_memory_stack* used;
        u32               stack_count; 
        u32               stack_size; 
    };

    struct map_memory : memory {
        map_memory_stack_list* stack_list;
    };

    //--------------------------------------------------------------------
    // MAP 
    //--------------------------------------------------------------------

    struct map {
        map_memory_stack* stack;
        map_chunk*        first_chunk;
    };
    
    //--------------------------------------------------------------------
    // MAP 
    //--------------------------------------------------------------------

    struct map_chunk {
        map*               map;
        map_chunk*         next;
        map_chunk*         prev;
        map_tile_color_u32 base_color;
        u32                count_rows;
        u32                count_cols;
        u32                offset_rows;
        u32                offset_cols;
    };
};

#endif //MAP_INTERNAL_HPP
