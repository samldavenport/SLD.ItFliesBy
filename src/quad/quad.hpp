#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb.hpp"
#include "entities.hpp"

namespace ifb {

    struct quad_manager {
        u32                  capacity;
        u32                  count;
        memory               reserved_memory;
        struct {
            entity_id*      id;
            vec3*           position;
            color_rgba_u32* color;
            size*           size;            
        } quad;
    };

    u32           quad_manager_memory_requirement (void);
    quad_manager* quad_manager_memory_init        (memory& mem);    
    void          quad_manager_startup            (quad_manager* qm, const memory& reserved_mem);
    
    bool          quad_create          (quad_manager* qm, const vec3* pos, const color_rgba_u32* color, const size* size);
    bool          quad_destroy         (quad_manager* qm, const quad_id id);
    bool          quad_update_position (quad_manager* qm, const quad_id id, const vec3* pos);
    bool          quad_update_color    (quad_manager* qm, const quad_id id, const color_rgba_u32* color);
    bool          quad_update_size     (quad_manager* qm, const quad_id id, const color_rgba_u32* size);
    bool          quad_get_entity      (quad_manager* qm, const quad_id id, entity_id* e_id);
    bool          quad_get_position    (quad_manager* qm, const quad_id id, vec3* pos);
    bool          quad_get_color       (quad_manager* qm, const quad_id id, color_rgba_u32* color);
    bool          quad_get_size        (quad_manager* qm, const quad_id id, color_rgba_u32* size);
};

#endif //IFB_QUAD_HPP