#ifndef QUAD_INTERNAL_HPP
#define QUAD_INTERNAL_HPP

#include "ifb-quad.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL bool quad_lookup_by_id  (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool quad_does_exist    (const entity_id    id);
    IFB_INTERNAL void quad_update        (const quad_entity& q);
    IFB_INTERNAL void quad_update        (const entity_id id, const cmpnt_quad& q);
    IFB_INTERNAL void quad_tests         (void);
    IFB_INTERNAL u32  quad_render_buffer (byte* buffer_data, const u32 buffer_size, const entity_list* quad_list, const orientation& camera_orientation);
    IFB_INTERNAL bool quad_render        (quad_vertices& vertices,    const entity_id    id,        const orientation& camera_orientation);
};

#endif //QUAD_INTERNAL_HPP
