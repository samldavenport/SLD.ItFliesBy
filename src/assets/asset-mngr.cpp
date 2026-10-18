#pragma once

#include "asset.hpp"
#include "asset-internal.hpp"
#include "eng-internal.hpp"

namespace ifb {

    struct asset_mngr {
        reservation*      res;
        asset_dictionary* dict;
        asset_memory*     mem;
    } static * _asset_mngr;

    IFB_INTERNAL asset_mngr*
    asset_mngr_create(
        void) {

        auto mngr = global_alloc<asset_mngr>();
        assert(mngr);

        mngr->res  = NULL;
        mngr->dict = NULL;
        mngr->mem  = asset_memory_create();
        assert(mngr->mem);

        _asset_mngr = mngr;
        return(_asset_mngr);
    }

    IFB_INTERNAL void
    asset_mngr_startup(
        reservation* res) {

        assert(_asset_mngr);
        assert(res);

        _asset_mngr->res = res;
        asset_memory_init(_asset_mngr->mem, res);
    }

    // called once at the start of every frame
    IFB_INTERNAL void
    asset_mngr_update(
        void) {

        assert(_asset_mngr);

        const u64 frame = eng_context_get_frame_count();
        asset_memory_update(_asset_mngr->mem, frame);
    }
};
