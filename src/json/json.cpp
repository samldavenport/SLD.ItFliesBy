#pragma once

#include <cassert>
#include <rapidjson/document.h>
#include "json.hpp"
#include "file-common.cpp"
#include "file-ro.cpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "rapidjson/allocators.h"
#include "rapidjson/encodings.h"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct json_allocator {
        arena* memory;
        static constexpr bool kNeedFree = false;

        void*
        Malloc(size_t size) {
            void* mem = arena_push(memory, size);
            return(mem);
        }

        void*
        Realloc(
            void*  original_ptr,
            size_t original_size,
            size_t new_size) {

            if (new_size == 0) {
                return(NULL);
            }

            void* new_ptr = arena_push(memory, new_size);

            if (original_ptr) {
                
                const u32 move_size = original_size < new_size
                    ? original_size
                    : new_size;

                (void)memmove(
                    new_ptr,
                    original_ptr,
                    move_size
                );
            }

            return(new_ptr);
        }
           
        static void
        Free(void* ptr) {
            //NOP
        }
    };

    using json_dom_allocator    = rapidjson::MemoryPoolAllocator<json_allocator>; 
    using json_generic_document = rapidjson::GenericDocument<rapidjson::UTF8<>, json_dom_allocator, json_allocator>;
    using json_itr_base         = rapidjson::Value::ValueIterator;
    using json_obj_base         = rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<ifb::json_allocator>>;
    using json_arr_base         = rapidjson::GenericArray<true, json_obj_base>;

    struct json_doc {
        json_allocator        allocator;
        json_dom_allocator    dom_allocator;
        json_generic_document base;
    };

    struct json_obj : json_obj_base { };
    struct json_arr : json_obj_base { };
    struct json_itr;

    //--------------------------------------------------------------------
    // DOCUMENT METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL json_doc*
    json_doc_create(
        arena*       a,
        const u32    json_cstr_length,
        const cchar* json_cstr_ptr) {

        assert(a                != NULL);
        assert(json_cstr_length != 0);
        assert(json_cstr_ptr    != NULL);
    
        auto doc = arena_push<json_doc>(a); 
        assert(doc != NULL);
  
        doc->allocator.memory = a;

        new (&doc->dom_allocator) json_dom_allocator(
            RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY,
            &doc->allocator
        );

        new (&doc->base)
        json_generic_document(
            &doc->dom_allocator,
            1024,
            &doc->allocator
        );  
      
        doc->base.Parse(json_cstr_ptr, json_cstr_length);

        return(doc);
    }
    
    IFB_INTERNAL void 
    json_doc_validate(
        const json_doc* doc) {

        assert(doc);
        assert(doc->base.IsObject());
    }
    
    IFB_INTERNAL const json_obj*
    json_doc_get_obj(
        const json_doc* doc,
        const cchar*    name) {

        json_doc_validate(doc);
        assert(name);

        auto member = doc->base.FindMember(name);

        if (member == doc->base.MemberEnd()) return(NULL);
        if (!member->value.IsObject())       return(NULL);

        return((const json_obj*)&member->value);
    }

    IFB_INTERNAL const json_arr*
    json_doc_get_arr(
        const json_doc* doc,
        const cchar*    name) {

        json_doc_validate(doc);
        assert(name);

        auto member = doc->base.FindMember(name);
        
        if (member == doc->base.MemberEnd()) return(NULL);
        if (!member->value.IsArray())        return(NULL);

        return((const json_arr*)&member->value);
    }

    IFB_INTERNAL const cchar*
    json_doc_get_cstr(
        const json_doc* doc,
        const cchar*         name) {

        json_doc_validate(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsString()
        );

        return(doc->base[name].GetString()); 
    }
   
    IFB_INTERNAL bool
    json_doc_get_bool(
        const json_doc* doc,
        const cchar*         name,
        bool&                val) {

        json_doc_validate(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsBool()
        );

        if (can_get) {
            val = doc->base[name].GetBool();             
        }

        return(can_get);
    }  

    IFB_INTERNAL bool
    json_doc_get_u32(
        const json_doc* doc,
        const cchar*         name,
        u32&                 val) {

        assert(doc);
        assert(name);

        bool can_get = true;
        can_get &= doc->base.HasMember(name); 
        can_get &= doc->base[name].IsUint();

        if (can_get) {
            val = doc->base[name].GetUint();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_doc_get_s32(
        const json_doc* doc,
        const cchar* name,
        s32&         val) {

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsInt()
        );

        if (can_get) {
            val = doc->base[name].GetInt();
        }

        return(can_get);
    } 

    IFB_INTERNAL bool
    json_doc_get_u64(
        const json_doc* doc,
        const cchar*         name,
        u64&                 val){

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsInt64()
        );

        if (can_get) {
            val = doc->base[name].GetUint64();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_doc_get_s64(
        const json_doc* doc,
        const cchar*         name,
        s64&                 val) {

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsInt64()
        );

        if (can_get) {
            val = doc->base[name].IsInt64();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_doc_get_f32(
        const json_doc* doc,
        const cchar* name,
        f32& val) {

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsFloat()
        );

        if (can_get) {
            val = doc->base[name].GetFloat();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_doc_get_f64(
        const json_doc* doc,
        const cchar* name,
        f64& val) {

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsDouble()
        );

        if (can_get) {
            val = doc->base[name].GetDouble();
        }

        return(can_get);
    }
    
    //--------------------------------------------------------------------
    // OBJECT METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const json_obj*
    json_obj_get_obj(
        const json_obj* obj,
        const cchar*       name) {

        assert(obj  != NULL);
        assert(name != NULL);

        auto member = obj->FindMember(name);

        if (member == obj->MemberEnd()) return(NULL);
        if (!member->value.IsObject())  return(NULL);

        return((const json_obj*)&member->value);
    }
    
    IFB_INTERNAL const cchar* 
    json_obj_get_cstr(
        const json_obj* obj,
        const cchar*    name) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsString()
        );

        return((*obj)[name].GetString());
    }

    IFB_INTERNAL bool
    json_obj_get_bool(
        const json_obj* obj,
        const cchar*    name,
        bool&           val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsBool()
        );

        if (can_get) {
            val = (*obj)[name].GetBool();
        }

        return(can_get);
    } 

    IFB_INTERNAL bool
    json_obj_get_u32(
        const json_obj* obj,
        const cchar*       name,
        u32&               val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsNumber()
        );

        if (can_get) {
            val = (*obj)[name].GetUint();
        }

        return(can_get);
    }
    
    IFB_INTERNAL bool
    json_obj_get_s32(
        const json_obj* obj,
        const cchar*       name,
        s32&               val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsNumber()
        );

        if (can_get) {
            val = (*obj)[name].GetInt();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_obj_get_u64(
        const json_obj* obj,
        const cchar*       name,
        u64&               val) {
   
        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsNumber()
        );

        if (can_get) {
            val = (*obj)[name].GetUint64();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_obj_get_s64(
        const json_obj* obj,
        const cchar*       name,
        s64&               val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsNumber()
        );

        if (can_get) {
            val = (*obj)[name].GetInt64();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_obj_get_f32(
        const json_obj* obj,
        const cchar*       name,
        f64&               val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsFloat()
        );

        if (can_get) {
            val = (*obj)[name].GetFloat();
        }

        return(can_get);
    }

    IFB_INTERNAL bool
    json_obj_get_f64(
        const json_obj* obj,
        const cchar*       name,
        f64&               val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsDouble()
        );

        if (can_get) {
            val = (*obj)[name].GetDouble();
        }

        return(can_get);
    }

    //--------------------------------------------------------------------
    // ARRAY METHODS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL u32
    json_arr_get_count(
        const json_arr* arr) {

        assert(arr && arr->IsArray());
        auto arr_base = arr->GetArray();

        return(arr_base.Size());
    }
    
    IFB_INTERNAL const json_obj*
    json_arr_get_obj(
        const json_arr* arr,
        const u32       index) {
    
        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto&     inst   = (*arr)[index]; 
        const bool      is_obj = inst.IsObject(); 
        const json_obj* obj    = is_obj ? (json_obj*)&inst : NULL;

        return(obj);
    }

    IFB_INTERNAL bool
    json_arr_get_bool(
        const json_arr* arr,
        const u32       index,
        bool&           val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst    = (*arr)[index]; 
        const bool  is_bool = inst.IsBool(); 

        if (is_bool) {
            val = inst.GetBool();
        }

        return(is_bool);
    } 

    IFB_INTERNAL bool
    json_arr_get_u32(
        const json_arr* arr,
        const u32       index,
        u32&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_u32 = inst.IsUint(); 

        if (is_u32) {
            val = inst.GetUint();
        }

        return(is_u32);
    }

    IFB_INTERNAL bool
    json_arr_get_s32(
        const json_arr* arr,
        const u32       index,
        s32&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_u32 = inst.IsInt(); 

        if (is_u32) {
            val = inst.GetInt();
        }

        return(is_u32);
    }

    IFB_INTERNAL bool
    json_arr_get_u64(
        const json_arr* arr,
        const u32       index,
        u64&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_u64 = inst.IsUint64(); 

        if (is_u64) {
            val = inst.GetUint64();
        }

        return(is_u64);
    }

    IFB_INTERNAL bool
    json_arr_get_s64(
        const json_arr* arr,
        const u32       index,
        s64&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_s64 = inst.IsInt64(); 

        if (is_s64) {
            val = inst.GetInt64();
        }

        return(is_s64);
    }

    IFB_INTERNAL bool
    json_arr_get_f32(
        const json_arr* arr,
        const u32       index,
        f64&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_f32 = inst.IsFloat(); 

        if (is_f32) {
            val = inst.GetFloat();
        }

        return(is_f32);
    }

    IFB_INTERNAL bool
    json_arr_get_f64(
        const json_arr* arr,
        const u32       index,
        f64&            val) {

        assert(arr && arr->IsArray()); 

        const u32 count = json_arr_get_count(arr);
        assert(index < count);

        const auto& inst   = (*arr)[index]; 
        const bool  is_f64 = inst.IsDouble(); 

        if (is_f64) {
            val = inst.GetDouble();
        }

        return(is_f64);
    }
   
    IFB_INTERNAL void
    json_test(
        void) {

        // get arena
        arena* a = arena_alloc();
        assert(a != NULL);

        // read the json data
        const file_handle json_hnd = file_ro_open_existing("test.json");
        const u32         size     = file_get_size(json_hnd);
        const cchar*      data     = file_read(json_hnd, size);    

        // create the document
        json_doc* doc = json_doc_create(a, size, data);
        bool result = true;
        u32 version;
        result &= json_doc_get_u32(doc, "version", version);
        assert(result);

        // read nested objects
        const json_obj* settings   = json_doc_get_obj(doc,      "settings");
        const json_obj* resolution = json_obj_get_obj(settings, "resolution");
        const json_obj* test_obj   = json_obj_get_obj(settings, "test-obj");
        assert(settings   != NULL);
        assert(resolution != NULL);
        assert(test_obj   == NULL);
       
        // get object values
        u32 width, height = 0;
        result &= json_obj_get_u32(resolution, "width",  width);
        result &= json_obj_get_u32(resolution, "height", height);
        assert(result);
        assert(width  == 1920);
        assert(height == 1080);

        // get string
        const char* name = json_doc_get_cstr(doc, "name"); 

        // get array count
        const json_arr* items = json_doc_get_arr(doc, "items"); 
        assert(items);
        const u32 items_count = json_arr_get_count(items);
        assert(items_count == 2);

        // get the second element
        const json_obj* item_1 = json_arr_get_obj(items, 1);
        assert(item_1);

        // get the second element properties 
        const cchar* item_1_name = json_obj_get_cstr(item_1, "name");
        assert(item_1_name != NULL);

        
        file_close(json_hnd);
        arena_free(a);
   
    }
};
