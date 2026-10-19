#pragma once

#include "eng-internal.hpp"
#include "memory.hpp"
#include "sprite-internal.hpp"
#include "sprites.hpp"

namespace ifb {

    static sprite_mngr* _sprite_mngr;

    IFB_INTERNAL sprite_mngr*
    sprite_mngr_create(
        void) {

        _sprite_mngr = (sprite_mngr*)global_alloc(sizeof(sprite_mngr)); 
        assert(_sprite_mngr);

        _sprite_mngr->mem = sprite_memory_create();
        assert(_sprite_mngr->mem);

        return(_sprite_mngr);
    }

    IFB_INTERNAL void
    sprite_mngr_startup(
        reservation* res) {

        assert(_sprite_mngr);
        assert(_sprite_mngr->mem);
        assert(res);

        sprite_memory_init(_sprite_mngr->mem, res);
    }
    
    IFB_INTERNAL sprite_memory*
    sprite_mngr_get_memory(
        void) {

        assert(_sprite_mngr);
        assert(_sprite_mngr->mem);

        return(_sprite_mngr->mem);
    }
};
