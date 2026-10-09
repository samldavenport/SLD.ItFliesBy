#pragma once

#include "ifb-collections.hpp"
#include "renderer-internal.hpp"
#include "renderer.cpp"

namespace ifb {

    struct render_lists {
        entity_list* quads;
        entity_list* particle_emitters;
    }; 

    IFB_INTERNAL render_lists*
    render_lists_create(
        void) {

        auto draw_lists = (render_lists*)global_alloc(sizeof(render_lists));
        assert(draw_lists);

        return(draw_lists);
    }

    IFB_INTERNAL void
    render_lists_init(
        render_lists* lists) {

        assert(lists);

        lists->quads             = renderer_context_create_entity_list(); 
        lists->particle_emitters = renderer_context_create_entity_list(); 

        assert(lists->quads);
        assert(lists->particle_emitters);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_quads(
        render_lists* lists) {

        assert(lists);
        assert(lists->quads);

        entity_list_validate(lists->quads);

        return(lists->quads);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_particle_emitters(
        render_lists* lists) {

        assert(lists);
        assert(lists->particle_emitters);

        entity_list_validate(lists->particle_emitters);

        return(lists->particle_emitters);
    }
};

