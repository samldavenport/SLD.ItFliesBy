#ifndef SPRITES_HPP
#define SPRITES_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct sprite_mngr;
    struct sprite; 
    struct sprite_atlas;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL sprite_mngr* sprite_mngr_create  (void);
    IFB_INTERNAL void         sprite_mngr_startup (void);

    IFB_INTERNAL sprite_atlas* sprite_atlas_create (u32 count_sprites); 
};

#endif //SPRITES_HPP
