#ifndef ASSET_INTERNAL_HPP
#define ASSET_INTERNAL_HPP

#include "asset-database.cpp"
#include "asset.hpp"
#include "ifb-types.hpp"
#include "json.cpp" //TODO(SAM): lets make this a header if possible
#include "sld.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // ASSET DICTIONARY 
    //--------------------------------------------------------------------
    
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
    };

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

    //--------------------------------------------------------------------
    // ASSET DATABASE 
    //--------------------------------------------------------------------
   
    IFB_INTERNAL u32  asset_database_get_asset_count (const asset_database* db);
    IFB_INTERNAL void asset_database_close           (const asset_database* db);

    //--------------------------------------------------------------------
    // ASSET MEMORY 
    //--------------------------------------------------------------------

    struct asset_memory_partition;
    struct asset_memory_block;
    struct asset_memory;

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

    struct asset_memory_block {
        u16 partition_index;
        u16 block_index;
    };
};

#endif //ASSET_INTERNAL_HPP
