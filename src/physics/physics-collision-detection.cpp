#pragma once

#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "ifb-types.hpp"
#include "map.hpp"
#include "memory-arena.cpp"
#include "physics-internal.hpp"
#include "entity.hpp"

namespace ifb {

    struct phys_cllsn_dtctr {
        phys_cllsn_list* cllsn_list;
    };

    IFB_INTERNAL phys_cllsn_dtctr*
    phys_cllsn_dtctr_create(
        void) {

        auto cllsn_dtctr = (phys_cllsn_dtctr*)phys_mngr_res_alloc(sizeof(phys_cllsn_dtctr));
        assert(cllsn_dtctr != NULL);

        cllsn_dtctr->cllsn_list = phys_cllsn_list_create();
        assert(cllsn_dtctr->cllsn_list != NULL);

        return(cllsn_dtctr);
    }

    IFB_INTERNAL void
    phys_cllsn_dtctr_run(
        phys_cllsn_dtctr* dtctr) {

        assert(dtctr             != NULL);
        assert(dtctr->cllsn_list != NULL);


        // allocate an arena
        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) return;

        const u32 map_count = map_mngr_get_count();                         

        for (
            u32 i_map = 0;
                i_map < map_count;
              ++i_map) {

              const hnd_map h_map = map_mngr_get_map(i_map);

              map_dimensions dims;
              map_get_dimensions(h_map, dims);
            
              for (
                  u32 i_chunk = 0;
                      i_chunk < dims.count_chunks;
                    ++i_chunk) {

                    const u32 save = arena_save(h_arena);

                    entity_list* list = map_get_entities_in_chunk(h_map, i_chunk, h_arena);
                    if (list == NULL) {
                        arena_revert(h_arena, save); 
                        continue; 
                    } 

                    const u32 entity_count = entity_list_count(list);
                    if (entity_count == 0) {
                        arena_revert(h_arena, save);
                        continue;
                    }

                  // build bounding box hierarchy / broad phase from list
                  //TODO(SLD)
              
                  arena_revert(h_arena, save);
              }
        }          

        arena_free(h_arena);
    }
};
