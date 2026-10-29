#ifndef SPRITES_HPP
#define SPRITES_HPP

#include "ifb.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct sprite_mngr;
    struct sprite_list;
    struct sprite_list_node;
    struct sprite_atlas_builder;
    struct sprite_atlas;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // sprite manager
    IFB_INTERNAL sprite_mngr*            sprite_mngr_create                           (void);
    IFB_INTERNAL void                    sprite_mngr_startup                          (reservation* res);
    IFB_INTERNAL hnd_sprite_atlas        sprite_mngr_load_atlas                       (const cchar* atlas_name_cstr);
    IFB_INTERNAL hnd_sprite_atlas        sprite_mngr_lookup_atlas                     (const cchar* atlas_name_cstr);
    IFB_INTERNAL bool                    sprite_mngr_unload_atlas                     (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL bool                    sprite_mngr_get_atlas_info                   (const hnd_sprite_atlas h_atlas, sprite_atlas_info& info);   
    IFB_INTERNAL hnd_sprite              sprite_mngr_lookup_sprite_handle             (const hnd_sprite_atlas h_atlas, const cchar*     sprite_name_cstr);
    IFB_INTERNAL u32                     sprite_mngr_lookup_sprite_index              (const hnd_sprite_atlas h_atlas, const cchar*     sprite_name_cstr);
    IFB_INTERNAL bool                    sprite_mngr_get_sprite_info_by_handle        (const hnd_sprite_atlas h_atlas, const hnd_sprite h_sprite, sprite_info& info);
    IFB_INTERNAL bool                    sprite_mngr_get_sprite_info_by_index         (const hnd_sprite_atlas h_atlas, const u32        i_sprite, sprite_info& info);
    IFB_INTERNAL image*                  sprite_mngr_get_sprite_image_by_handle       (const hnd_sprite_atlas h_atlas, const hnd_sprite h_sprite, const hnd_arena h_arena);
    IFB_INTERNAL image*                  sprite_mngr_get_sprite_image_by_index        (const hnd_sprite_atlas h_atlas, const u32        i_sprite, const hnd_arena h_arena);
    IFB_INTERNAL image*                  sprite_mngr_get_sprite_atlas_image_by_handle (const hnd_sprite_atlas h_atlas, const hnd_arena h_arena);

    // sprite atlas
    IFB_INTERNAL sprite_atlas*           sprite_atlas_create                     (void);
    IFB_INTERNAL void                    sprite_atlas_destroy                    (sprite_atlas* atlas);
    IFB_INTERNAL bool                    sprite_atlas_build                      (sprite_atlas* atlas, sprite_atlas_builder* builder);
    IFB_INTERNAL bool                    sprite_atlas_save_to_file               (sprite_atlas* atlas, const cchar* path);
    IFB_INTERNAL bool                    sprite_atlas_load_from_file             (sprite_atlas* atlas, const cchar* path); 
    IFB_INTERNAL sprite_atlas*           sprite_atlas_load_from_asset_db         (sprite_atlas* atlas, const cchar* name);
    IFB_INTERNAL u32                     sprite_atlas_get_sprite_count           (const sprite_atlas* atlas);
    IFB_INTERNAL const image*            sprite_atlas_get_image                  (const sprite_atlas* atlas);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_info            (const sprite_atlas* atlas, const u32 index, sprite_atlas_sprite_info& info);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_name         (const sprite_atlas* atlas, const cchar*     name);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_handle       (const sprite_atlas* atlas, const hnd_sprite hnd);
    IFB_INTERNAL bool                    sprite_atlas_get_sprite_by_index        (const sprite_atlas* atlas, const u32        index);

    // sprite builder
    IFB_INTERNAL sprite_atlas_builder*   sprite_atlas_builder_create             (void);
    IFB_INTERNAL void                    sprite_atlas_builder_destroy            (sprite_atlas_builder* builder);
    IFB_INTERNAL void                    sprite_atlas_builder_reset              (sprite_atlas_builder* builder);
    IFB_INTERNAL void                    sprite_atlas_builder_set_list           (sprite_atlas_builder* builder, sprite_list* list);
    IFB_INTERNAL bool                    sprite_atlas_builder_pack_sprites       (sprite_atlas_builder* builder);
    IFB_INTERNAL const image*            sprite_atlas_builder_get_image          (sprite_atlas_builder* builder);
    IFB_INTERNAL const sprite_list*      sprite_atlas_builder_get_sprite_list    (sprite_atlas_builder* builder);
    IFB_INTERNAL u32                     sprite_atlas_builder_get_sprite_offset  (sprite_atlas_builder* builder, const u32 index);

    // sprite list
    IFB_INTERNAL sprite_list*            sprite_list_create                      (const hnd_arena h_arena);
    IFB_INTERNAL bool                    sprite_list_add                         (sprite_list* list, const cchar* asset_img_name, const hnd_arena h_arena);
    IFB_INTERNAL bool                    sprite_list_remove                      (sprite_list* list, const cchar* asset_img_name);
    IFB_INTERNAL const sprite_list_node* sprite_list_get_first                   (const sprite_list* list); 
    IFB_INTERNAL u32                     sprite_list_get_count                   (const sprite_list* list);
    IFB_INTERNAL const sprite_list_node* sprite_list_get_next                    (const sprite_list_node* node);
    IFB_INTERNAL hnd_asset               sprite_list_get_img_asset_hnd           (const sprite_list_node* node);
    IFB_INTERNAL const dimensions_2d&    sprite_list_get_img_dims                (const sprite_list_node* node);
    IFB_INTERNAL hnd_sprite_atlas        sprite_list_save_to_atlas               (const sprite_list* list, const cchar* atlas, const hnd_arena h_arena);
};

#endif //SPRITES_HPP
