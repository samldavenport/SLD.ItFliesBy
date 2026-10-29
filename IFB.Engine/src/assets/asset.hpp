#ifndef ASSETS_HPP
#define ASSETS_HPP

#include "ifb.hpp"
#include "sld.hpp"
#include "memory-internal.hpp"
#include "types-internal.hpp"
#include "eng-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct sprite_index;
    struct asset_text {
        hnd_asset hnd;
        u32       size;
        cchar*    cstr;
    };

    struct asset_image {
        hnd_asset       hnd;
        u32             width;
        u32             height;
        color_rgba_u32* pixels;
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
    };

    struct asset_sprite_atlas {
        hnd_sprite*      array_handles;
        sprite_index*    array_indexes;
        pixel*           array_pixels;
        hnd_sprite_atlas hnd;
        u32              width;
        u32              height;
        u32              sprite_count;
    };

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct asset_mngr;
    struct asset_dictionary;
    struct asset_database;
    
    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // asset
    IFB_INTERNAL hnd_asset          asset_name_cstr_to_handle          (const cchar* name_cstr);

    // manager
    IFB_INTERNAL asset_mngr*               asset_mngr_create                   (void);
    IFB_INTERNAL void                      asset_mngr_startup                  (reservation* res);
    IFB_INTERNAL void                      asset_mngr_update                   (void);
    IFB_INTERNAL void                      asset_mngr_shutdown                 (void);
    IFB_INTERNAL bool                      asset_mngr_open_database            (void);
    IFB_INTERNAL bool                      asset_mngr_close_database           (void);
    IFB_INTERNAL hnd_asset                 asset_mngr_load_text                (const cchar* name);
    IFB_INTERNAL hnd_asset                 asset_mngr_load_image               (const cchar* name);
    IFB_INTERNAL hnd_asset                 asset_mngr_load_sprite_atlas        (const cchar* name);
    IFB_INTERNAL const asset_text*         asset_mngr_get_text                 (const hnd_asset hnd);
    IFB_INTERNAL const asset_image*        asset_mngr_get_image                (const hnd_asset hnd);
    IFB_INTERNAL const asset_sprite_atlas* asset_mngr_get_sprite_atlas         (const hnd_asset hnd);
    IFB_INTERNAL const asset_name*         asset_mngr_get_name                 (const hnd_asset hnd);
    IFB_INTERNAL asset_database*           asset_mngr_get_database             (void);
    

    // dictionary
    IFB_INTERNAL asset_dictionary*  asset_dictionary_create                (const hnd_arena arena);
    IFB_INTERNAL asset_dictionary*  asset_dictionary_load_from_json        (const cchar*    path,const hnd_arena h_arena);
    IFB_INTERNAL bool               asset_dictionary_save_to_json          (asset_dictionary* dict, const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL hnd_asset          asset_dictionary_add_text_file         (asset_dictionary* dict, const cchar* name, const cchar* path, const hnd_arena h_arena); 
    IFB_INTERNAL hnd_asset          asset_dictionary_add_image_file        (asset_dictionary* dict, const cchar* name, const cchar* path, const hnd_arena h_arena); 
    IFB_INTERNAL hnd_asset          asset_dictionary_add_sprite_atlas_file (asset_dictionary* dict, const cchar* name, const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL bool               asset_dictionary_remove_file           (asset_dictionary* dict, const cchar* name);
    IFB_INTERNAL u32                asset_dictionary_count_assets          (const asset_dictionary* dict);

    // database
    IFB_INTERNAL asset_database*    asset_database_load                           (const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL asset_database*    asset_database_build                          (const cchar* path, const asset_dictionary* dict, const hnd_arena);
    IFB_INTERNAL bool               asset_database_get_index                      (const asset_database* db, const cchar* name_cstr, asset_index& index);
    IFB_INTERNAL const asset_name*  asset_database_get_name                       (const asset_database* db, const hnd_asset h_asset);
    IFB_INTERNAL const asset_text*  asset_database_load_text_by_handle_to_arena   (const asset_database* db, const hnd_asset h_asset,   const hnd_arena h_arena);
    IFB_INTERNAL const asset_text*  asset_database_load_text_by_name_to_arena     (const asset_database* db, const cchar*    name_cstr, const hnd_arena h_arena);
    IFB_INTERNAL const asset_image* asset_database_load_image_by_handle_to_arena  (const asset_database* db, const hnd_asset h_asset,   const hnd_arena h_arena);
    IFB_INTERNAL const asset_image* asset_database_load_image_by_name_to_arena    (const asset_database* db, const cchar*    name_cstr, const hnd_arena h_arena);
    IFB_INTERNAL const asset_text*  asset_database_load_text_by_handle_to_memory  (const asset_database* db, const hnd_asset h_asset,   memory& mem);
    IFB_INTERNAL const asset_text*  asset_database_load_text_by_name_to_memory    (const asset_database* db, const cchar*    name_cstr, memory& mem);
    IFB_INTERNAL const asset_image* asset_database_load_image_by_handle_to_memory (const asset_database* db, const hnd_asset h_asset,   memory& mem);
    IFB_INTERNAL const asset_image* asset_database_load_image_by_name_to_memory   (const asset_database* db, const cchar*    name_cstr, memory& mem);
    IFB_INTERNAL const asset_sprite_atlas* asset_database_load_sprite_atlas_by_handle_to_arena  (const asset_database* db, const hnd_asset h_asset,   const hnd_arena h_arena);
    IFB_INTERNAL const asset_sprite_atlas* asset_database_load_sprite_atlas_by_name_to_arena    (const asset_database* db, const cchar*    name_cstr, const hnd_arena h_arena);
    IFB_INTERNAL const asset_sprite_atlas* asset_database_load_sprite_atlas_by_handle_to_memory (const asset_database* db, const hnd_asset h_asset,   memory& mem);
    IFB_INTERNAL const asset_sprite_atlas* asset_database_load_sprite_atlas_by_name_to_memory   (const asset_database* db, const cchar*    name_cstr, memory& mem);
    IFB_INTERNAL const asset_sprite_atlas* asset_database_load_sprite_atlas_file                (const cchar* path, const hnd_arena h_arena);
};

#endif //ASSETS_HPP
