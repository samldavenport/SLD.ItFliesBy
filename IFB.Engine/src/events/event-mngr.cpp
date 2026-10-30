#pragma once

#include <cassert>

#include "eng-context.hpp"
#include "eng-events.hpp"
#include "eng-memory.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    struct event_mngr {
        reservation*          res;
        event_queue*          queue;
        event_listener_table* listener_table;
    } static * _event_mngr;
    
    IFB_INTERNAL event_mngr*
    event_mngr_create(
        void) {

        _event_mngr = (event_mngr*)global_alloc(sizeof(event_mngr));
        assert(_event_mngr);

        return(_event_mngr);
    }

    IFB_INTERNAL void
    event_mngr_startup(
        reservation* res) {

        assert(_event_mngr);

        _event_mngr->res = res;

        _event_mngr->queue          = event_queue_create();
        _event_mngr->listener_table = event_listener_table_create();
   
        assert(_event_mngr->queue);
        assert(_event_mngr->listener_table);
    }

    IFB_INTERNAL void
    event_mngr_shutdown(
        void) {

        //TODO(SLD)
    }

    IFB_INTERNAL void
    event_mngr_update(
        void) {

        assert(_event_mngr);
        assert(_event_mngr->queue);
        assert(_event_mngr->listener_table);

        event_queue_execute_and_reset(
            _event_mngr->queue,
            _event_mngr->listener_table
        );
    }
    
    IFB_INTERNAL void*
    event_mngr_alloc(
        const u32 size) {

        assert(size             != 0);
        assert(_event_mngr      != NULL);
        assert(_event_mngr->res != NULL);

        void* mem = reservation_push_bytes(_event_mngr->res, size);

        return(mem);
    }
    
    IFB_INTERNAL bool
    event_mngr_publish(
        const event_type_u32 type,
        const event_payload& payload) {
    
        assert(_event_mngr);
        assert(_event_mngr->queue);
        assert(_event_mngr->listener_table);
        assert(type.val < config::event_type_max);
    
        const bool did_push = event_queue_push(_event_mngr->queue, type, payload);
        return(did_push);
    }
    
    IFB_INTERNAL bool
    event_mngr_subscribe(
        const event_type_u32      type,
        const fptr_event_listener listener) {

        assert(_event_mngr);
        assert(_event_mngr->queue);
        assert(_event_mngr->listener_table);
    
        const bool did_add = event_listener_table_add(_event_mngr->listener_table, type, listener);
        return(did_add);
    }

    IFB_INTERNAL bool
    event_mngr_unsubscribe(
        const event_type_u32      type,
        const fptr_event_listener listener) {

        assert(_event_mngr);
        assert(_event_mngr->queue);
        assert(_event_mngr->listener_table);
    
        const bool did_remmove = event_listener_table_remove(_event_mngr->listener_table, type, listener);
        return(did_remmove);
    }
};
