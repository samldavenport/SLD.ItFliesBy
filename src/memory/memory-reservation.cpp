#pragma once

#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "memory.hpp"
#include "eng-internal.hpp"
#include "sld.hpp"
#include <cassert>

namespace ifb {
   
    inline void
    reservation_validate(
        const reservation* res) {

        assert(res);
        assert(res->start         != 0);
        assert(res->page_capacity != 0);
        assert(res->page_count    <= res->page_capacity);
    }

    IFB_INTERNAL reservation*
    reservation_create(
        const memory& mem) {

        auto res = global_alloc<reservation>(); 

        assert(res);
        assert(mem.size    != 0);
        assert(mem.address != 0);

        const u32 page_size = system_get_memory_page_size();
        assert(size_is_pow_2(page_size));
        assert(page_size > 0);

        res->start         = mem.ptr;
        res->page_count    = 0; 
        res->page_capacity = mem.size / page_size; 
        assert(res->page_capacity > 0);

        return(res);
    }

    IFB_INTERNAL void*
    reservation_push_bytes(
        reservation* res,
        const u32    size_min) {

        reservation_validate(res);
        assert(size_min != 0);

        const auto& cfg = config_instance();

        // get the page and aligned sizes
        const u32 size_aligned = system_align_to_memory_page_size(size_min);
        const u32 page_size    = system_get_memory_page_size();
        assert(size_aligned != 0);
        assert(page_size    != 0);

        // calculate the number of pages to push
        const u32 page_count_push = size_aligned / page_size;
        assert(page_count_push != 0);

        // make sure we can push these pages
        const u32 page_count_new = res->page_count + page_count_push;
        if (page_count_new > res->page_capacity) return(NULL);

        // commit memory
        const u32 commit_offset = res->page_count * page_size;
        void*     cmt           = pfm_memory_commit(res->start, commit_offset, size_aligned);
        assert(cmt != NULL);

        // update the reservation and
        // return the committed memory
        res->page_count = page_count_new;
        return(cmt);
    }

    IFB_INTERNAL void*
    reservation_push_pages(
        reservation* res, 
        const u32    page_count) {

        reservation_validate(res);
        assert(page_count != 0);

        // calculate the aligned sizes
        const u32 size_page    = system_get_memory_page_size(); 
        const u32 size_aligned = page_count * size_page;
        assert(size_page    != 0);
        assert(size_aligned != 0);

        // make sure we can push these pages
        const u32 page_count_new = res->page_count + page_count;
        if (page_count_new > res->page_count) return(NULL);

        // commit memory
        const u32 commit_offset = res->page_count * size_page;
        void* cmt = pfm_memory_commit(res->start, commit_offset, size_aligned);
        assert(cmt != NULL);

        // update the reservation and
        // return the committed memory
        res->page_count = page_count_new;
        return(cmt);
    } 

    IFB_INTERNAL stack*
    reservation_push_stack_bytes(
        reservation* res,
        const u32    size_min) {

        reservation_validate(res);
        assert(size_min != 0);

        // get the page and aligned sizes
        const u32 size_aligned = system_align_to_memory_page_size(size_min);
        const u32 page_size    = system_get_memory_page_size();
        assert(size_aligned != 0);
        assert(page_size    != 0);

        // calculate the number of pages to push
        const u32 page_count_push = size_aligned / page_size;
        assert(page_count_push != 0);

        // make sure we can push these pages
        const u32 page_count_new = res->page_count + page_count_push;
        if (page_count_new > res->page_count) return(NULL);

        // commit memory
        const u32 commit_offset = res->page_count * page_size;
        void* cmt = pfm_memory_commit(res->start, commit_offset, size_aligned);
        assert(cmt != NULL);

        // update the reservation
        res->page_count = page_count_new;
    
        // create the stack
        auto s = (stack*)cmt;
        memory stack_mem;
        stack_mem.address = (addr)cmt    + sizeof(stack);
        stack_mem.size    = size_aligned - sizeof(stack);
        s->init(stack_mem);
        return(s);
    }

    IFB_INTERNAL stack*
    reservation_push_stack_pages(
        reservation* res,
        const u32    page_count) {

        reservation_validate(res);
        assert(page_count != 0);

        // calculate the aligned sizes
        const u32 size_page    = system_get_memory_page_size(); 
        const u32 size_aligned = page_count * size_page;
        assert(size_page    != 0);
        assert(size_aligned != 0);

        // make sure we can push these pages
        const u32 page_count_new = res->page_count + page_count;
        if (page_count_new > res->page_count) return(NULL);

        // commit memory
        const u32 commit_offset = res->page_count * size_page;
        void* cmt = pfm_memory_commit(res->start, commit_offset, size_aligned);
        assert(cmt != NULL);

        // update the reservation
        res->page_count = page_count_new;
    
        // create the stack
        auto s = (stack*)cmt;
        memory stack_mem;
        stack_mem.address = (addr)cmt    + sizeof(stack);
        stack_mem.size    = size_aligned - sizeof(stack);
        s->init(stack_mem);
        return(s);
    }
    
    IFB_INTERNAL void*
    reservation_push_all(
        reservation* res) {

        reservation_validate(res);

        void* mem = reservation_push_pages(res, res->page_capacity);
        return(mem);
    }
    
    IFB_INTERNAL void
    reservation_decommit(
        reservation* res) {

        reservation_validate(res);

        const u32 size_page    = system_get_memory_page_size();
        const u32 size_deommit = size_page * res->page_count;
        pfm_memory_decommit(res->start, size_deommit);
    }
};
