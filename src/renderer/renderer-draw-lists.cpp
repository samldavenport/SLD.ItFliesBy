#pragma once

#include "ifb-collections.hpp"
#include "renderer-internal.hpp"
#include "renderer.cpp"

namespace ifb {

    struct render_draw_lists {
        entity_list* quads;
        entity_list* particle_emitters;
    }; 

    IFB_INTERNAL render_draw_lists*
    render_lists_create(
        void) {

        auto draw_lists = (render_draw_lists*)global_alloc(sizeof(render_draw_lists));
        assert(draw_lists);

        return(draw_lists);
    }

    IFB_INTERNAL void
    render_lists_init(
        render_draw_lists* draw_lists) {

        assert(draw_lists);

        draw_lists->quads             = renderer_context_create_entity_list(); 
        draw_lists->particle_emitters = renderer_context_create_entity_list(); 

        assert(draw_lists->quads);
        assert(draw_lists->particle_emitters);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_quads(
        render_draw_lists* draw_lists) {

        assert(draw_lists);
        assert(draw_lists->quads);

        entity_list_validate(draw_lists->quads);

        return(draw_lists->quads);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_particle_emitters(
        render_draw_lists* draw_lists) {

        assert(draw_lists);
        assert(draw_lists->particle_emitters);

        entity_list_validate(draw_lists->particle_emitters);

        return(draw_lists->particle_emitters);
    }
};

