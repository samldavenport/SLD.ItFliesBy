#ifndef RENDERER_INTERNAL_HPP
#define RENDERER_INTERNAL_HPP

#include "renderer.hpp"

namespace ifb {

    struct render_buffers;
    struct render_lists;
    struct render_gl;

    IFB_INTERNAL void*              renderer_context_memory_alloc       (const u32 size);
    IFB_INTERNAL entity_list*       renderer_context_create_entity_list (void);
    
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
