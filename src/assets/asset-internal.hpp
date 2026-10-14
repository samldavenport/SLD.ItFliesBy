#ifndef ASSET_INTERNAL_HPP
#define ASSET_INTERNAL_HPP

#include "asset.hpp"
#include "ifb-types.hpp"
#include "json.cpp" //TODO(SAM): lets make this a header if possible

namespace ifb {
    
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

};

#endif //ASSET_INTERNAL_HPP
