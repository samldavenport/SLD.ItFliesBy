#pragma once

#include "sprite-internal.hpp"
#include "sprites.hpp"

namespace ifb {

    static sprite_mngr* _sprite_mngr;

    IFB_INTERNAL sprite_mngr*
    sprite_mngr_create(
        void) {

        _sprite_mngr = (sprite_mngr*)global_alloc(sizeof(sprite_mngr)); 
        assert(_sprite_mngr);

        return(_sprite_mngr);
    }

    IFB_INTERNAL void
    sprite_mngr_startup(
        reservation* res) {

        assert(_sprite_mngr);
        assert(res);

        _sprite_mngr->res = res;
    }
};
