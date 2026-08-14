#pragma once

#include "cartographer.hpp"
#include "cartographer-internal.cpp"
#include "cartographer-memory.cpp"

namespace ifb {

    IFB_INTERNAL map*
    cartographer_map_create(
        void) {
    
        map* m = cartographer_memory_alloc_map(); 
        if (m == NULL) return(NULL);

        assert(_cartographer != NULL);

        if (_cartographer->first_map == NULL) {
            _cartographer->first_map = m;
        }
        else {
            auto next_map = _cartographer->first_map;
            next_map->prev = m;
            m->next = next_map;
            _cartographer->first_map = m;
        }

        return(m);
    }    

    IFB_INTERNAL void
    cartographer_map_destroy(
        map* m) {

        assert(_cartographer);
        assert(_cartographer->first_map);
        assert(m);

        for (
            tile* t = m->tile_list;
            t != NULL;
            t = t->next) {

            cartographer_tile_destroy(t); 
        }
    
        auto m_prev = m->prev;
        auto m_next = m->next;
        
        if (m == _cartographer->first_map) {
            assert(m_prev == NULL);
            _cartographer->first_map = m_next;
        }
        else if (m_prev) {
            assert(m != _cartographer->first_map);
            m_prev->next = m_next;
        }

        if (m_next) {
            m_next->prev = m_prev;
        }

        m->next = NULL;
        m->prev = NULL;

        cartographer_memory_free_map(m);
    }

    IFB_INTERNAL void
    cartographer_map_add_tile(
        map*  m,
        tile* t) {

        assert(m);
        assert(t);

        for (
            tile* curr = m->tile_list;
            curr != NULL; 
            curr = curr->next) {

            assert(t != curr);
        }

        tile* next = m->tile_list;
        if (next) next->prev = t;
        t->next = next;
        t->prev = NULL;
        t->map  = m;
        m->tile_list = t;
    }

    IFB_INTERNAL void
    cartographer_map_remove_tile(
        map*  m,
        tile* t) {

        assert(m);
        assert(t);
        assert(t->map == m);

        tile* prev = t->prev;
        tile* next = t->next;

        if (prev)              prev->next   = next;
        if (next)              next->prev   = prev;
        if (t == m->tile_list) m->tile_list = next;

        t->next = NULL;
        t->prev = NULL;
        t->map  = NULL;
    }
    
    IFB_INTERNAL void
    cartographer_map_set_root_tile(
        map* m,
        tile* t) {

        assert(m);
        assert(t);


        bool should_add = true;
        for (
            tile* curr = m->tile_list;
            curr != NULL; 
            curr = curr->next) {

            if (t == curr) {
                should_add = false;
                break;
            }
        }

        if (should_add) {

            tile* next = m->tile_list;
            assert(next->prev == NULL);

            t->next      = next;
            t->prev      = NULL;
            next->prev   = t;
            m->tile_list = t;
        }

        m->root_tile = t;
    }
};
