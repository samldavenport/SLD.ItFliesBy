#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "ifb-entities.hpp"
#include "sld.hpp"
#include "ifb-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_vertex;
    struct quad_tile;
    struct quad_vertices;
    struct quad_vertex_buffer;
    
    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack* stack_mem;
    };

    struct quad_vertex {
        vec3           position;
        color_rgba_f32 color;
        vec2           text_coords;
        f32            textured;     // 0 is a solid color, 1 samples the sprite atlas
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
