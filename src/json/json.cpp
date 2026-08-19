#pragma once

#include <cassert>
#include <rapidjson/document.h>
#include "json.hpp"
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

    struct json_document {
        json_allocator        allocator;
        json_dom_allocator    dom_allocator;
        json_generic_document base;
    };

    struct json_object {
        json_allocator allocator;
        json_iter* iter;    
    };

    IFB_INTERNAL json_document*
    json_document_create(
        arena*       a,
        const u32    json_cstr_length,
        const cchar* json_cstr_ptr) {

        assert(a                != NULL);
        assert(json_cstr_length != 0);
        assert(json_cstr_ptr    != NULL);
    
        auto doc = arena_push<json_document>(a); 
        assert(doc != NULL);
   
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
       
        return(doc);
    }
    
    IFB_INTERNAL void 
    json_doc_validate(
        const json_document* doc) {

        assert(doc);
        assert(doc->base.IsObject());
    }
    
    IFB_INTERNAL json_object*
    json_doc_get_object(
        const json_document* doc,
        const cchar*         name) {

        json_doc_validate(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
        );

          = doc->base[name].GetObject();
        }


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

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsUint()
        );

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
  
    IFB_INTERNAL json_iterator*
    json_iter_get_first(
        const json_document* doc,
        const cchar*         name) {

        assert(doc);
        assert(name);

        const bool can_get = (
            doc->base.HasMember(name) &&
            doc->base[name].IsArray()
        );

        if (can_get) {
            val = doc->base[name].Begin();
        }

        return(can_get);
        
    }
 
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
};
