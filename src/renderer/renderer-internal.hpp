#ifndef RENDERER_INTERNAL_HPP
#define RENDERER_INTERNAL_HPP

#include "ifb-types.hpp"
#include "renderer.hpp"

namespace ifb {

    struct render_buffers;
    struct render_lists;
    struct render_gl;

    IFB_INTERNAL void*              renderer_context_memory_alloc       (const u32 size);
    IFB_INTERNAL entity_list*       renderer_context_create_entity_list (void);
   
    IFB_INTERNAL renderer_camera*   renderer_camera_create              (void);
    IFB_INTERNAL void               renderer_camera_init                (renderer_camera* camera);
    IFB_INTERNAL const vec3&        renderer_camera_get_origin          (renderer_camera* camera);
    IFB_INTERNAL const vec3&        renderer_camera_get_target          (renderer_camera* camera);
    IFB_INTERNAL void               renderer_camera_set_origin          (renderer_camera* camera, const vec3& origin);
    IFB_INTERNAL void               renderer_camera_set_target          (renderer_camera* camera, const vec3& target);
    IFB_INTERNAL void               renderer_camera_get_orientation     (renderer_camera* camera, orientation& ori);
    IFB_INTERNAL void               renderer_camera_get_xform           (renderer_camera* camera, mat4& xform);

    IFB_INTERNAL render_buffers*    render_buffers_create               (void);
    IFB_INTERNAL void               render_buffers_alloc_and_init       (render_buffers* buffers);
    IFB_INTERNAL void               render_buffers_render_quads         (render_buffers* buffers, render_lists* lists);
    IFB_INTERNAL void               render_buffers_render_tiles         (render_buffers* buffers, render_lists* lists);

    IFB_INTERNAL render_lists*      render_lists_create                 (void);
    IFB_INTERNAL void               render_lists_init                   (render_lists* lists);
    IFB_INTERNAL void               render_lists_reset                  (render_lists* lists);
    IFB_INTERNAL const entity_list* render_lists_get_quads              (render_lists* lists);
    IFB_INTERNAL const entity_list* render_lists_get_particle_emitters  (render_lists* lists);
};

#endif //RENDERER_INTERNAL_HPP
