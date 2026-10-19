#ifndef SPRITES_HPP
#define SPRITES_HPP

#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct sprite_mngr;
    struct sprite_list;
    struct sprite_list_node;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // sprite manager
    IFB_INTERNAL sprite_mngr*            sprite_mngr_create                      (void);
    IFB_INTERNAL void                    sprite_mngr_startup                     (reservation* res);

    // sprite atlas
    IFB_INTERNAL hnd_sprite_atlas        sprite_atlas_load_from_file             (const cchar* path); 
    IFB_INTERNAL hnd_sprite_atlas        sprite_atlas_load_from_asset_db         (const cchar* name);
    IFB_INTERNAL u32                     sprite_atlas_get_sprite_count           (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL u32                     sprite_atlas_get_pixel_width            (const hnd_sprite_atlas h_atlas); 
    IFB_INTERNAL u32                     sprite_atlas_get_pixel_height           (const hnd_sprite_atlas h_atlas); 
    IFB_INTERNAL const pixel*            sprite_atlas_get_pixel_array            (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL const dimensions_2d&    sprite_atlas_get_sprite_dimensions      (const hnd_sprite_atlas h_atlas, const u32 index);  
    IFB_INTERNAL const cchar*            sprite_atlas_get_sprite_asset_name_cstr (const hnd_sprite_atlas h_atlas, const u32 index);  
    IFB_INTERNAL hnd_asset               sprite_atlas_get_sprite_asset_hnd       (const hnd_sprite_atlas h_atlas, const u32 index);
    IFB_INTERNAL u32                     sprite_atlas_get_sprite_offset          (const hnd_sprite_atlas h_atlas, const u32 index);

    // sprite list
    IFB_INTERNAL sprite_list*            sprite_list_create                      (const hnd_arena h_arena);
    IFB_INTERNAL bool                    sprite_list_add                         (sprite_list* list, const cchar* asset_img_name, const hnd_arena h_arena);
    IFB_INTERNAL bool                    sprite_list_remove                      (sprite_list* list, const cchar* asset_img_name);
    IFB_INTERNAL const sprite_list_node* sprite_list_get_first                   (const sprite_list* list); 
    IFB_INTERNAL const sprite_list_node* sprite_list_get_next                    (const sprite_list_node* node);
    IFB_INTERNAL hnd_asset               sprite_list_get_img_asset_hnd           (const sprite_list_node* node);
    IFB_INTERNAL const dimensions_2d&    sprite_list_get_img_dims                (const sprite_list_node* node);
    IFB_INTERNAL hnd_sprite_atlas        sprite_list_save_to_atlas               (const sprite_list* list, const cchar* atlas, const hnd_arena h_arena); 

};

#endif //SPRITES_HPP
