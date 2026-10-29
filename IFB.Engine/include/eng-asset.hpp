#ifndef ENG_ASSET_HPP
#define ENG_ASSET_HPP

#include "asset-database.cpp"
#include "ifb.hpp"
#include "json.cpp" //TODO(SAM): lets make this a header if possible
#include "sld.hpp"
#include "eng-sprite.hpp"
#include "eng-json.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct asset_mngr;
    struct asset_dictionary;
    struct asset_database;
    struct asset_memory_partition;
    struct asset_memory_block;
    struct asset_memory;
    struct asset_dictionary_file_list;
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

    IFB_INTERNAL asset_memory* asset_memory_create        (void);
    IFB_INTERNAL void          asset_memory_init          (asset_memory* asset_mem, reservation* res);
    IFB_INTERNAL bool          asset_memory_alloc         (asset_memory* asset_mem, const hnd_asset hnd, const u32 size, asset_memory_block& block);
    IFB_INTERNAL void          asset_memory_free          (asset_memory* asset_mem, const asset_memory_block& block);
    IFB_INTERNAL void          asset_memory_update        (asset_memory* asset_mem, const u64 frame);
    IFB_INTERNAL u32           asset_memory_count_pinned  (const asset_memory* asset_mem);
    IFB_INTERNAL bool          asset_memory_mark_for_free (asset_memory* asset_mem, const asset_memory_block& block, const hnd_asset hnd);
    IFB_INTERNAL addr          asset_memory_get_data      (asset_memory* asset_mem, const asset_memory_block& block, const hnd_asset hnd);
    IFB_INTERNAL addr          asset_memory_pin           (asset_memory* asset_mem, const asset_memory_block& block, const hnd_asset hnd);
    IFB_INTERNAL void          asset_memory_unpin         (asset_memory* asset_mem, const asset_memory_block& block, const hnd_asset hnd);

    
    IFB_INTERNAL const asset_dictionary_file_list*
    asset_dictionary_get_list(
        const asset_dictionary* dict,
        const asset_type_u32    type
    );

    IFB_INTERNAL u32       asset_dictionary_json_write_cstr          (cchar* buffer, const u32 offset,const cchar* cstr); 
    IFB_INTERNAL u32       asset_dictionary_file_list_json_size      (const asset_dictionary_file_list& list); 
    IFB_INTERNAL bool      asset_dictionary_file_list_load_from_json (asset_dictionary_file_list& list,const json_arr* arr,const hnd_arena h_arena);
    IFB_INTERNAL bool      asset_dictionary_file_list_remove_node    (asset_dictionary_file_list& list,const u32 id); 
    IFB_INTERNAL u32       asset_dictionary_json_write_cstr          (cchar* buffer,const u32 offset, const cchar* cstr); 
    IFB_INTERNAL u32       asset_dictionary_file_list_json_size      (const asset_dictionary_file_list& list); 
    IFB_INTERNAL bool      asset_dictionary_file_list_load_from_json (asset_dictionary_file_list& list,const json_arr* arr,const hnd_arena h_arena);
    IFB_INTERNAL bool      asset_dictionary_file_list_remove_node    (asset_dictionary_file_list& list, const u32 id);
    IFB_INTERNAL hnd_asset asset_dictionary_file_list_add_node       (asset_dictionary_file_list& list, const cchar* name, const cchar* path, const hnd_arena h_arena);
    IFB_INTERNAL u32       asset_dictionary_name_to_upper            (const cchar* name, cchar* name_upper);
    IFB_INTERNAL u32       asset_dictionary_file_list_json_write     (const asset_dictionary_file_list& list, const cchar* name, const bool is_last, cchar* buffer, const u32 offset); 
    IFB_INTERNAL u32       asset_dictionary_file_list_count          (const asset_dictionary_file_list& list);

    IFB_INTERNAL u32  asset_database_get_asset_count (const asset_database* db);
    IFB_INTERNAL void asset_database_close           (const asset_database* db);

    
    struct asset_dictionary_file_node {
        asset_dictionary_file_node* next;
        const cchar*                path;
        const cchar*                name;
        u32                         id;
    };

    struct asset_dictionary_file_list {
        asset_dictionary_file_node* first;
        asset_dictionary_file_node* last;
    };

    struct asset_dictionary {
        asset_dictionary_file_list text_files;
        asset_dictionary_file_list image_files;
        asset_dictionary_file_list sprite_atlas_files;
    };

    struct asset_memory_block {
        u16 partition_index;
        u16 block_index;
    };
};

#endif //ENG_ASSET_HPP
