#ifndef ENG_QUAD_HPP
#define ENG_QUAD_HPP

#include "ifb.hpp"
#include "sld.hpp"

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

    // what a quad's fragments sample, the quad shader picks with the vertex's textured value
    static constexpr f32 QUAD_SAMPLE_SOLID  = 0.0f; // the vertex color
    static constexpr f32 QUAD_SAMPLE_SPRITE = 1.0f; // the active sprite atlas, tinted by the vertex color
    static constexpr f32 QUAD_SAMPLE_FONT   = 2.0f; // the active font's coverage as the alpha of the vertex color

    struct quad_vertex {
        vec3           position;
        color_rgba_f32 color;
        vec2           text_coords;
        f32            textured;     // one of QUAD_SAMPLE_*
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

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL bool quad_lookup_by_id  (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool quad_does_exist    (const entity_id    id);
    IFB_INTERNAL void quad_update        (const quad_entity& q);
    IFB_INTERNAL void quad_update        (const entity_id id, const cmpnt_quad& q);
    IFB_INTERNAL u32  quad_render_buffer (byte* buffer_data, const u32 buffer_size, const entity_list* quad_list, const orientation& camera_orientation);
    IFB_INTERNAL bool quad_render        (quad_vertices& vertices,    const entity_id    id,        const orientation& camera_orientation);
};

#endif //ENG_QUAD_HPP
