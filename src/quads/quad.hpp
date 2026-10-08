#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "sld.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_vertex;
    struct quad_tile;
    struct quad_vertices;
    struct quad_vertex_buffer;

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL bool quad_lookup_by_id  (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool quad_does_exist    (const entity_id    id);
    IFB_INTERNAL void quad_update        (const quad_entity& q);
    IFB_INTERNAL void quad_update        (const entity_id id, const cmpnt_quad& q);
    IFB_INTERNAL void quad_tests         (void);
    IFB_INTERNAL u32  quad_render_buffer (quad_vertex_buffer& buffer, const entity_list* quad_list, const orientation& camera_orientation, const hnd_arena h_arena);
    IFB_INTERNAL void quad_render        (quad_vertices& vertices,    const entity_id    id,        const orientation& camera_orientation);
    
    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack* stack_mem;
    };

    struct quad_vertex {
        vec3           position;
        color_rgba_f32 color;
    };

    struct quad_vertices {
        quad_vertex top_right;
        quad_vertex bottom_right;
        quad_vertex bottom_left;
        quad_vertex top_left;
    };

    struct quad_vertex_buffer {
        u32 size;
        union {
            quad_vertices* vertices;
            byte*          bytes;
            void*          vptr;
            f32*           floats;
            addr           addr;
        } data;
    };
};


#endif //IFB_QUAD_HPP
