#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "memory.hpp"
#include "entities.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct component_mngr;
    struct component_table_position;
    struct component_table_color;
    struct component_list_position;
    struct component_list_color;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static component_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL component_mngr* component_mngr_create  (void);
    IFB_INTERNAL void            component_mngr_startup (memory& mem_res);

    IFB_INTERNAL component_list_position* component_position_table_lookup (arena* in_arena, const entity_list* entt_list,   const u32 count = 1);
    IFB_INTERNAL void                     component_position_list_get    (const component_list_position*    list, const u32 index, component_position&       pos);
    IFB_INTERNAL void                     component_position_list_update (component_list_position*          list, const u32 index, const component_position& pos);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct component_mngr {
        stack mem;
        u32   capacity;
        struct  {
            component_table_position* position;
            component_table_color*    color; 
        } tables;
    };

    struct component_table_position {
        f32* x;
        f32* y;
        f32* z;
    };

    struct component_table_color {
        u32* rgba_hex;
    };

    struct component_list_position {
        struct {
            entity_id* id;
            f32*       x;
            f32*       y;
            f32*       z;            
        } data;
        u32 capacity;
        u32 count;
    };

    struct component_list_color {
        color_rgba_u32* data;
        u32             capacity;
        u32             count;

    };
};

#endif //COMPONENTS_HPP