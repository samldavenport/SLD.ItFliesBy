#ifndef ENG_JSON_HPP
#define ENG_JSON_HPP

#include "ifb.hpp"
#include "sld.hpp"

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
    
    IFB_INTERNAL json_doc*       json_doc_create   (const hnd_arena arena_hnd, const u32 json_cstr_length, const cchar* json_cstr_ptr);
    IFB_INTERNAL void            json_doc_validate (const json_doc* doc);
    IFB_INTERNAL const json_obj* json_doc_get_root (const json_doc* doc);
    IFB_INTERNAL const json_obj* json_doc_get_obj  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL const json_arr* json_doc_get_arr  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL const cchar*    json_doc_get_cstr (const json_doc* doc, const cchar* name);
    IFB_INTERNAL bool            json_doc_get_bool (const json_doc* doc, const cchar* name); 
    IFB_INTERNAL u32             json_doc_get_u32  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL s32             json_doc_get_s32  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL u64             json_doc_get_u64  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL s64             json_doc_get_s64  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL f32             json_doc_get_f32  (const json_doc* doc, const cchar* name);
    IFB_INTERNAL f64             json_doc_get_f64  (const json_doc* doc, const cchar* name);
    
    //--------------------------------------------------------------------
    // OBJECT METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const json_obj* json_obj_get_obj  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL const json_arr* json_obj_get_arr  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL const cchar*    json_obj_get_cstr (const json_obj* obj, const cchar* name); 
    IFB_INTERNAL bool            json_obj_get_bool (const json_obj* obj, const cchar* name); 
    IFB_INTERNAL u32             json_obj_get_u32  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL s32             json_obj_get_s32  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL u64             json_obj_get_u64  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL s64             json_obj_get_s64  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL f32             json_obj_get_f32  (const json_obj* obj, const cchar* name);
    IFB_INTERNAL f64             json_obj_get_f64  (const json_obj* obj, const cchar* name);

    // these return false instead of asserting when the member is missing or the wrong type
    IFB_INTERNAL bool            json_obj_has_member   (const json_obj* obj, const cchar* name);
    IFB_INTERNAL bool            json_obj_try_get_u32  (const json_obj* obj, const cchar* name, u32& val);
    IFB_INTERNAL bool            json_obj_try_get_cstr (const json_obj* obj, const cchar* name, const cchar*& val);

    //--------------------------------------------------------------------
    // ARRAY METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL u32             json_arr_get_count (const json_arr* arr);
    IFB_INTERNAL const json_obj* json_arr_get_obj   (const json_arr* arr, const u32 index); 
    IFB_INTERNAL const cchar*    json_arr_get_cstr  (const json_arr* arr, const u32 index);
    IFB_INTERNAL bool            json_arr_get_bool  (const json_arr* arr, const u32 index); 
    IFB_INTERNAL u32             json_arr_get_u32   (const json_arr* arr, const u32 index);
    IFB_INTERNAL s32             json_arr_get_s32   (const json_arr* arr, const u32 index);
    IFB_INTERNAL u64             json_arr_get_u64   (const json_arr* arr, const u32 index);
    IFB_INTERNAL s64             json_arr_get_s64   (const json_arr* arr, const u32 index);
    IFB_INTERNAL f32             json_arr_get_f32   (const json_arr* arr, const u32 index);
    IFB_INTERNAL f64             json_arr_get_f64   (const json_arr* arr, const u32 index);

    // these return false instead of asserting when the index is out of range or the value is the wrong type
    IFB_INTERNAL bool            json_arr_try_get_s32  (const json_arr* arr, const u32 index, s32& val);
    IFB_INTERNAL bool            json_arr_try_get_f32  (const json_arr* arr, const u32 index, f32& val);
    IFB_INTERNAL bool            json_arr_try_get_cstr (const json_arr* arr, const u32 index, const cchar*& val);

    //--------------------------------------------------------------------
    // TEST METHODS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void            json_test          (void);
};

#endif //ENG_JSON_HPP
