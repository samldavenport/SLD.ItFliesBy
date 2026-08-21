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
    using json_iter             = rapidjson::Value::ConstValueIterator;
    using json_object_base      = rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<ifb::json_allocator>>;
    using json_array_base       = json_generic_document::ConstArray;

    struct json_document {
        json_allocator        allocator;
        json_dom_allocator    dom_allocator;
        json_generic_document base;
    };

    struct json_object : json_object_base { };
    struct json_array  : json_array_base  { };

    IFB_INTERNAL json_document*
    json_doc_create(
        arena*       a,
        const u32    json_cstr_length,
        const cchar* json_cstr_ptr) {

        assert(a                != NULL);
        assert(json_cstr_length != 0);
        assert(json_cstr_ptr    != NULL);
    
        auto doc = arena_push<json_document>(a); 
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
        const json_document* doc) {

        assert(doc);
        assert(doc->base.IsObject());
    }
    
    IFB_INTERNAL const json_object*
    json_doc_get_object(
        const json_document* doc,
        const cchar*         name) {

        json_doc_validate(doc);
        assert(name);

        auto member = doc->base.FindMember(name);

        if (member == doc->base.MemberEnd()) return(NULL);
        if (!member->value.IsObject())       return(NULL);

        return((const json_object*)&member->value);
    }

    IFB_INTERNAL const json_array*
    json_doc_get_array(
        const json_document* doc,
        const cchar*         name) {

        json_doc_validate(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name),
            doc->base[name].IsArray()
        );
            
        return(NULL);
    }

    IFB_INTERNAL const cchar*
    json_doc_get_string(
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
        const json_document* doc,
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
    
    IFB_INTERNAL const json_object*
    json_object_get_object(
        const json_object* obj,
        const cchar*       name) {

        assert(obj  != NULL);
        assert(name != NULL);

        auto member = obj->FindMember(name);

        if (member == obj->MemberEnd()) return(NULL);
        if (!member->value.IsObject())  return(NULL);

        return((const json_object*)&member->value);
    }

    IFB_INTERNAL bool
    json_object_get_string_length(
        const json_object* obj,
        const cchar*       name,
        u32&               val) {

        assert(obj  != NULL);
        assert(name != NULL);
        
        return(false);
    }

    IFB_INTERNAL bool
    json_object_get_string_val(
        const json_object* obj,
        const cchar*       name,
        const cchar*&      val) {

        assert(obj  != NULL);
        assert(name != NULL);

        const bool can_get = (
            obj->HasMember(name) &&
            (*obj)[name].IsString()
        );

        if (can_get) {
            val = (*obj)[name].GetString();
        }

        return(can_get);

    }

    IFB_INTERNAL bool
    json_object_get_bool(
        const json_object* obj,
        const cchar*       name,
        bool&              val) {

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
    json_object_get_u32(
        const json_object* obj,
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
    json_object_get_s32(
        const json_object* obj,
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
    json_object_get_u64(
        const json_object* obj,
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
    json_object_get_s64(
        const json_object* obj,
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
    json_object_get_f32(
        const json_object* obj,
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
    json_object_get_f64(
        const json_object* obj,
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

    IFB_INTERNAL void
    json_test(
        void) {

        arena* a = arena_alloc();
        assert(a != NULL);

        const file_handle json_hnd = file_ro_open_existing("test.json");
        const u32         size     = file_get_size(json_hnd);
        const cchar*      data     = file_read(json_hnd, size);    

        json_document* doc = json_doc_create(a, size, data);
        bool result = true;
        u32 version;
        result &= json_doc_get_u32(doc, "version", version);
        assert(result);

        const json_object* settings   = json_doc_get_object    (doc,      "settings");
        const json_object* resolution = json_object_get_object (settings, "resolution");
        const json_object* test_obj   = json_object_get_object (settings, "test-obj");
        assert(settings   != NULL);
        assert(resolution != NULL);
        assert(test_obj   == NULL);
        
        u32 width, height = 0;
        result &= json_object_get_u32(resolution, "width",  width);
        result &= json_object_get_u32(resolution, "height", height);
        assert(result);
        assert(width  == 1920);
        assert(height == 1080);

        const char* name = json_doc_get_string(doc, "name"); 

        file_close(json_hnd);
        arena_free(a);
   
    }
};
