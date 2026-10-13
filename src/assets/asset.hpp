#ifndef ASSETS_HPP
#define ASSETS_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "ifb.hpp"
#include "memory-reservation.cpp"
#include "sld.hpp"
#include <sld-strings.hpp>

namespace ifb {

    struct asset_mngr;
    struct asset_dictionary;
    struct asset_database;
    struct asset_index;
    struct asset_name;

    IFB_INTERNAL asset_mngr*       asset_mngr_create                   (void);
    IFB_INTERNAL void              asset_mngr_init                     (asset_mngr* mngr, reservation* res);
    IFB_INTERNAL bool              asset_mngr_load_database            (asset_mngr* mngr, asset_database* db);
    IFB_INTERNAL hnd_asset         asset_mngr_get_handle               (const cchar* name);

    IFB_INTERNAL asset_dictionary* asset_dictionary_create             (const hnd_arena arena);
    IFB_INTERNAL void              asset_dictionary_file_load          (asset_database* db, const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL void              asset_dictionary_file_build         (asset_database* db, const cchar* path, const asset_dictionary* dict, const hnd_arena h_arena);

    IFB_INTERNAL asset_database*   asset_database_create               (const hnd_arena h_arena);
    IFB_INTERNAL asset_database*   asset_database_load                 (const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL bool              asset_database_init_from_dictionary (const asset_dictionary* dict, const hnd_arena h_arena);
    IFB_INTERNAL bool              asset_database_save                 (const asset_database* db, const cchar* path); 
};

#endif //ASSETS_HPP
