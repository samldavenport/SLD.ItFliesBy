#pragma once

#include "asset.hpp"

namespace ifb {

    struct asset_mngr {
        reservation*      res;
        asset_dictionary* dict;
    };

    IFB_INTERNAL asset_mngr*
    asset_mngr_create(
        void) {

    }

    IFB_INTERNAL void
    asset_mngr_startup(
        asset_mngr*  mngr,
        reservation* res) {

    }
};
