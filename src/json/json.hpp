#ifndef JSON_HPP
#define JSON_HPP

#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct json_doc;
    struct json_itr;
    struct json_obj; 
    struct json_arr;
    
    //--------------------------------------------------------------------
    // DOCUMENT METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL json_doc*       json_doc_create   (arena* a, const u32 json_cstr_length, const cchar* json_cstr_ptr);
    IFB_INTERNAL void            json_doc_validate (const json_doc* doc);         
    IFB_INTERNAL const json_obj* json_doc_get_obj  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL const json_arr* json_doc_get_arr  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL const cchar*    json_doc_get_cstr (const json_doc* doc, const cchar* name);
    IFB_INTERNAL bool            json_doc_get_bool (const json_doc* doc, const cchar* name, bool&   val); 
    IFB_INTERNAL bool            json_doc_get_u32  (const json_doc* doc, const cchar* name, u32& val);
    IFB_INTERNAL bool            json_doc_get_s32  (const json_doc* doc, const cchar* name, s32& val);
    IFB_INTERNAL bool            json_doc_get_u64  (const json_doc* doc, const cchar* name, u64& val);
    IFB_INTERNAL bool            json_doc_get_s64  (const json_doc* doc, const cchar* name, s64& val);
    IFB_INTERNAL bool            json_doc_get_f32  (const json_doc* doc, const cchar* name, f64& val);
    IFB_INTERNAL bool            json_doc_get_f64  (const json_doc* doc, const cchar* name, f64& val);
    
    //--------------------------------------------------------------------
    // OBJECT METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const json_obj* json_obj_get_obj  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL const cchar*    json_obj_get_cstr (const json_obj* obj, const cchar* name); 
    IFB_INTERNAL bool            json_obj_get_bool (const json_obj* obj, const cchar* name, bool&   val); 
    IFB_INTERNAL bool            json_obj_get_u32  (const json_obj* obj, const cchar* name, u32& val);
    IFB_INTERNAL bool            json_obj_get_s32  (const json_obj* obj, const cchar* name, s32& val);
    IFB_INTERNAL bool            json_obj_get_u64  (const json_obj* obj, const cchar* name, u64& val);
    IFB_INTERNAL bool            json_obj_get_s64  (const json_obj* obj, const cchar* name, s64& val);
    IFB_INTERNAL bool            json_obj_get_f32  (const json_obj* obj, const cchar* name, f64& val);
    IFB_INTERNAL bool            json_obj_get_f64  (const json_obj* obj, const cchar* name, f64& val);

    //--------------------------------------------------------------------
    // ARRAY METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL u32             json_arr_get_count (const json_arr* arr);
    IFB_INTERNAL const json_obj* json_arr_get_obj   (const json_arr* arr, const u32 index); 
    IFB_INTERNAL bool            json_arr_get_bool  (const json_arr* arr, const u32 index, bool&   val); 
    IFB_INTERNAL bool            json_arr_get_u32   (const json_arr* arr, const u32 index, u32& val);
    IFB_INTERNAL bool            json_arr_get_s32   (const json_arr* arr, const u32 index, s32& val);
    IFB_INTERNAL bool            json_arr_get_u64   (const json_arr* arr, const u32 index, u64& val);
    IFB_INTERNAL bool            json_arr_get_s64   (const json_arr* arr, const u32 index, s64& val);
    IFB_INTERNAL bool            json_arr_get_f32   (const json_arr* arr, const u32 index, f64& val);
    IFB_INTERNAL bool            json_arr_get_f64   (const json_arr* arr, const u32 index, f64& val);

    //--------------------------------------------------------------------
    // TEST METHODS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void            json_test          (void);
};

#endif //JSON_HPP
