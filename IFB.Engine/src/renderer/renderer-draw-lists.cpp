#pragma once

#include "eng-renderer.hpp"
#include "eng-context.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-map.hpp"
#include "eng-quad.hpp"

namespace ifb {

    struct render_lists {
        entity_list* quads;
        entity_list* particle_emitters;
        hnd_map      map;
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

        lists->quads             = renderer_create_entity_list(); 
        lists->particle_emitters = renderer_create_entity_list(); 

        assert(lists->quads);
        assert(lists->particle_emitters);

        lists->map = INVALID_HANDLE;
    }

    IFB_INTERNAL void
    render_lists_reset(
        render_lists* lists) {

        assert(lists);
        assert(lists->quads);
        assert(lists->particle_emitters);
    
        // the map stays set until it's changed,
        // everything else gets pushed again every frame
        eng_entity_list_reset(lists->quads);
        eng_entity_list_reset(lists->particle_emitters);
    }

    IFB_INTERNAL bool
    render_lists_push_quad(
        render_lists*   lists,
        const entity_id id) {

        assert(lists);
        assert(lists->quads);
        assert(id != ENTITY_ID_INVALID);

        const bool did_push = quad_does_exist(id) && eng_entity_list_add(lists->quads, id);
        return(did_push);
    }

    IFB_INTERNAL bool
    render_lists_push_particle_emitter(
        render_lists*   lists,
        const entity_id id) {

        assert(lists);
        assert(lists->particle_emitters);
        assert(id != ENTITY_ID_INVALID);

        const bool did_push = eng_entity_list_add(lists->particle_emitters, id);
        return(did_push);
    }

    IFB_INTERNAL bool
    render_lists_set_map(
        render_lists* lists,
        const hnd_map h_map) {

        assert(lists);
        assert(h_map != INVALID_HANDLE);

        // make sure the map exists
        map_dimensions dims;
        const bool did_set = map_get_dimensions(h_map, dims);
        if (did_set) {
            lists->map = h_map;
        }
        return(did_set);
    }

    IFB_INTERNAL hnd_map
    render_lists_get_map(
        render_lists* lists) {

        assert(lists);
        return(lists->map);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_quads(
        render_lists* lists) {

        assert(lists);
        assert(lists->quads);

        eng_entity_list_validate(lists->quads);

        return(lists->quads);
    }

    IFB_INTERNAL const entity_list*
    render_lists_get_particle_emitters(
        render_lists* lists) {

        assert(lists);
        assert(lists->particle_emitters);

        eng_entity_list_validate(lists->particle_emitters);

        return(lists->particle_emitters);
    }
};

