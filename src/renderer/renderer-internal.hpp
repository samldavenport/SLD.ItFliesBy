#ifndef RENDERER_INTERNAL_HPP
#define RENDERER_INTERNAL_HPP

#include "renderer.hpp"

namespace ifb {

    struct render_buffers;
    struct render_draw_lists;

    IFB_INTERNAL render_buffers*    render_buffers_create                   (void);
    IFB_INTERNAL void               render_buffers_alloc_and_init           (render_buffers* buffers);
    IFB_INTERNAL void               render_buffers_render_quads             (render_buffers* buffers);
    IFB_INTERNAL void               render_buffers_render_tiles             (render_buffers* buffers);

    IFB_INTERNAL render_draw_lists* render_draw_lists_create                (void);
    IFB_INTERNAL void               render_draw_lists_init                  (render_draw_lists* draw_lists);
    IFB_INTERNAL const entity_list* render_draw_lists_get_quads             (render_draw_lists* draw_lists);
    IFB_INTERNAL const entity_list* render_draw_lists_get_particle_emitters (render_draw_lists* draw_lists);
};

#endif //RENDERER_INTERNAL_HPP
