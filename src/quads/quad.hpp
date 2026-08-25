#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_mngr;
    struct quad_entity;
    struct quad_vertex;
    struct quad_tile;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static quad_mngr* _quad_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL quad_mngr* quad_mngr_create    (void);
    IFB_INTERNAL void       quad_mngr_validate  (void);
    IFB_INTERNAL void       quad_mngr_startup   (memory& mem);

    IFB_INTERNAL bool       quad_lookup_by_id   (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool       quad_does_exist     (const entity_id    id);
    IFB_INTERNAL void       quad_update         (const quad_entity& q);
    IFB_INTERNAL void       quad_update         (const entity_id id, const quad& q);

    IFB_INTERNAL void       quad_tests          (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack stack_mem;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };

    struct quad_tile_id {
        union {
            struct {
                u32 grid_id;
                u16 row_index;
                u16 col_index;
            };
            u32 val;
        };
    };

    struct quad_tile {
        quad_tile_id   id;
        color_rgba_u32 color; 
        u32            texture_coord_x;
        u32            texture_coord_y;
    };

    struct quad_tile_grid_id {
        u32 val;
    };

    struct quad_tile_map_id {
        u32 val;
    };

    struct quad_tile_grid {
        quad_tile_grid_id grid_id;
        quad_tile_map_id  map_id;
        u32               count_rows;
        u32               count_cols;
        quad_tile*        tile_array;
    };

    struct quad_tile_map {

    };
};

#endif //IFB_QUAD_HPP
