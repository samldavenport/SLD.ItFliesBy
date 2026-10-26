#pragma once

#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "ifb-types.hpp"
#include "physics-internal.hpp"
#include "entity.hpp"

namespace ifb {

    struct phys_cllsn_dtctr {
        entity_list* list;
    };

    IFB_INTERNAL phys_cllsn_dtctr*
    phys_cllsn_dtctr_create(
        void) {

        auto cllsn_dtctr = (phys_cllsn_dtctr*)phys_mngr_res_alloc(sizeof(phys_cllsn_dtctr));
        assert(cllsn_dtctr != NULL);

        memory list_mem;
        list_mem.size = entity_list_mem_req();
        list_mem.ptr  = phys_mngr_res_alloc(list_mem.size); 
        assert(list_mem.ptr != NULL);

        cllsn_dtctr->list = entity_list_memory_create(list_mem);
        assert(cllsn_dtctr->list != NULL);

        return(cllsn_dtctr);
    }

    IFB_INTERNAL void
    phys_cllsn_dtctr_run(
        phys_cllsn_dtctr* dtctr) {

        assert(dtctr       != NULL);
        assert(dtctr->list != NULL);

                                 
    }
};
