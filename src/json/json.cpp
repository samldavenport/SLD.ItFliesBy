#pragma once

#include <rapidjson/document.h>
#include "json.hpp"
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

    struct json_document {
        json_allocator        allocator;
        json_dom_allocator    dom_allocator;
        json_generic_document base;
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

    }

    IFB_INTERNAL bool
    json_doc_get_f64(
        const json_document* doc,
        const cchar* name,
        f64& val);
};
