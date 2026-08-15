#ifndef JSON_HPP
#define JSON_HPP

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    struct json_document;

    IFB_INTERNAL json_document* json_doc_create            (arena* a, const u32 json_cstr_length, const cchar* json_cstr_ptr);
    IFB_INTERNAL void           json_doc_validate          (const json_document* doc);         
    IFB_INTERNAL bool           json_doc_get_string_length (const json_document* doc, const cchar* name, u32&    val);
    IFB_INTERNAL bool           json_doc_get_string_val    (const json_document* doc, const cchar* name, cchar*& val);
    IFB_INTERNAL bool           json_doc_get_bool          (const json_document* doc, const cchar* name, bool&   val); 
    IFB_INTERNAL bool           json_doc_get_u32           (const json_document* doc, const cchar* name, u32& val);
    IFB_INTERNAL bool           json_doc_get_s32           (const json_document* doc, const cchar* name, s32& val);
    IFB_INTERNAL bool           json_doc_get_u64           (const json_document* doc, const cchar* name, u64& val);
    IFB_INTERNAL bool           json_doc_get_s64           (const json_document* doc, const cchar* name, s64& val);
    IFB_INTERNAL bool           json_doc_get_f64           (const json_document* doc, const cchar* name, f64& val);
    
};

#endif //JSON_HPP
