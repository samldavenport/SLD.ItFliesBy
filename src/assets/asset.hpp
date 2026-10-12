#ifndef ASSETS_HPP
#define ASSETS_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    struct asset_mngr;
    struct asset_dictionary;
    struct asset_database;

    IFB_INTERNAL asset_mngr*       asset_mngr_create                   (void);
    IFB_INTERNAL void              asset_mngr_init                     (asset_mngr* mngr, reservation* res);
    IFB_INTERNAL bool              asset_mngr_load_database            (asset_mngr* mngr, asset_database* db);
    IFB_INTERNAL hnd_asset         asset_mngr_get_handle               (const cchar* name);

    IFB_INTERNAL asset_dictionary* asset_dictionary_create             (const hnd_arena arena);
    IFB_INTERNAL asset_dictionary* asset_dictionary_load_from_json     (const cchar* path, const hnd_arena arena);
    IFB_INTERNAL bool              asset_dictionary_save_to_json       (asset_dictionary* dict, const cchar* path, const hnd_arena arena);
    IFB_INTERNAL hnd_asset         asset_dictionary_add_text_file      (asset_dictionary* dict, const cchar* name, const cchar* path, const hnd_arena arena);
    IFB_INTERNAL hnd_asset         asset_dictionary_add_image_file     (asset_dictionary* dict, const cchar* name, const cchar* path, const hnd_arena arena);
    IFB_INTERNAL bool              asset_dictionary_remove_file        (asset_dictionary* dict, const cchar* name);

    IFB_INTERNAL asset_database*   asset_database_create               (const hnd_arena h_arena);
    IFB_INTERNAL asset_database*   asset_database_load                 (const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL bool              asset_database_init_from_dictionary (const asset_dictionary* dict, const hnd_arena h_arena);
    IFB_INTERNAL bool              asset_database_save                 (const asset_database* db, const cchar* path); 
};

#endif //ASSETS_HPP
