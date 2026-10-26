#pragma once

#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "ifb-types.hpp"
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

                                 
    }
};
