#ifndef JSON_HPP
#define JSON_HPP

#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {

    struct json_document;
    struct json_iterator;
    struct json_object; 
    struct json_array;
    
    IFB_INTERNAL json_document*     json_doc_create               (arena* a, const u32 json_cstr_length, const cchar* json_cstr_ptr);
    IFB_INTERNAL void               json_doc_validate             (const json_document* doc);         
    IFB_INTERNAL const json_object* json_doc_get_object           (const json_document* doc, const cchar* name, u32&    val);
    IFB_INTERNAL const json_array*  json_doc_get_array            (const json_document* doc, const cchar* name);
    IFB_INTERNAL const cchar*       json_doc_get_string           (const json_document* doc, const cchar* name);
    IFB_INTERNAL bool               json_doc_get_bool             (const json_document* doc, const cchar* name, bool&   val); 
    IFB_INTERNAL bool               json_doc_get_u32              (const json_document* doc, const cchar* name, u32& val);
    IFB_INTERNAL bool               json_doc_get_s32              (const json_document* doc, const cchar* name, s32& val);
    IFB_INTERNAL bool               json_doc_get_u64              (const json_document* doc, const cchar* name, u64& val);
    IFB_INTERNAL bool               json_doc_get_s64              (const json_document* doc, const cchar* name, s64& val);
    IFB_INTERNAL bool               json_doc_get_f32              (const json_document* doc, const cchar* name, f64& val);
    IFB_INTERNAL bool               json_doc_get_f64              (const json_document* doc, const cchar* name, f64& val);
    
    IFB_INTERNAL const json_object* json_object_get_object        (const json_object* obj, const cchar* name);
    IFB_INTERNAL bool               json_object_get_string_length (const json_object* obj, const cchar* name, u32&    val);
    IFB_INTERNAL bool               json_object_get_string_val    (const json_object* obj, const cchar* name, cchar*& val);
    IFB_INTERNAL bool               json_object_get_bool          (const json_object* obj, const cchar* name, bool&   val); 
    IFB_INTERNAL bool               json_object_get_u32           (const json_object* obj, const cchar* name, u32& val);
    IFB_INTERNAL bool               json_object_get_s32           (const json_object* obj, const cchar* name, s32& val);
    IFB_INTERNAL bool               json_object_get_u64           (const json_object* obj, const cchar* name, u64& val);
    IFB_INTERNAL bool               json_object_get_s64           (const json_object* obj, const cchar* name, s64& val);
    IFB_INTERNAL bool               json_object_get_f32           (const json_object* obj, const cchar* name, f64& val);
    IFB_INTERNAL bool               json_object_get_f64           (const json_object* obj, const cchar* name, f64& val);


    IFB_INTERNAL json_iterator* json_iter_get_first           (const json_document* doc, const cchar* name);
    IFB_INTERNAL json_iterator* json_iter_get_next            (const json_iterator* next);
    IFB_INTERNAL json_object*   json_iter_get_object          (const json_iterator* iter, const cchar* name, u32&    val);
    IFB_INTERNAL bool           json_iter_get_string_length   (const json_iterator* iter, const cchar* name, u32&    val);
    IFB_INTERNAL bool           json_iter_get_string_val      (const json_iterator* iter, const cchar* name, cchar*& val);
    IFB_INTERNAL bool           json_iter_get_bool            (const json_iterator* iter, const cchar* name, bool&   val); 
    IFB_INTERNAL bool           json_iter_get_u32             (const json_iterator* iter, const cchar* name, u32& val);
    IFB_INTERNAL bool           json_iter_get_s32             (const json_iterator* iter, const cchar* name, s32& val);
    IFB_INTERNAL bool           json_iter_get_u64             (const json_iterator* iter, const cchar* name, u64& val);
    IFB_INTERNAL bool           json_iter_get_s64             (const json_iterator* iter, const cchar* name, s64& val);
    IFB_INTERNAL bool           json_iter_get_f32             (const json_iterator* iter, const cchar* name, f64& val);
    IFB_INTERNAL bool           json_iter_get_f64             (const json_iterator* iter, const cchar* name, f64& val);


    IFB_INTERNAL void           json_test                     (void);
};

#endif //JSON_HPP
