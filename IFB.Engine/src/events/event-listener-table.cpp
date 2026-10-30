#pragma once

#include <cassert>

#include "eng-events.hpp"
#include "ifb.hpp"

namespace ifb {

    struct event_listener_table {
        u32                 array_count     [config::event_type_max];
        fptr_event_listener array_listeners [config::event_type_max][config::event_max_listeners_per_event];
    };


    IFB_INTERNAL event_listener_table*
    event_listener_table_create(
        void) {

        auto tbl = (event_listener_table*)event_mngr_alloc(sizeof(event_listener_table));
        assert(tbl);

        for (
            u32 type = 0;
                type < config::event_type_max;
              ++type) {

            tbl->array_count[type] = 0;
        }

        return(tbl);
    } 

    IFB_INTERNAL bool
    event_listener_table_add(
        event_listener_table*     listener_table,
        const event_type_u32      type,
        const fptr_event_listener listener) {

        assert(listener_table);
        assert(type.val < config::event_type_max);
        assert(listener);

        const u32 count = listener_table->array_count[type.val];  
        assert(count <= config::event_max_listeners_per_event);
    
        if (count == config::event_max_listeners_per_event) {
            return(false);
        }

        u32& index       = listener_table->array_count     [type.val];
        auto listeners   = listener_table->array_listeners [type.val];
        listeners[index] = listener;
        ++index; 

        return(true);
    }

    IFB_INTERNAL bool
    event_listener_table_remove(
        event_listener_table*     listener_table,
        const event_type_u32      type,
        const fptr_event_listener listener) {

        assert(listener_table);
        assert(type.val < config::event_type_max);
        assert(listener);

        u32& count = listener_table->array_count[type.val];  
        assert(count <= config::event_max_listeners_per_event);
    
        if (count == 0) {
            return(false);
        }

        const u32 last      = count - 1;
        auto      listeners = listener_table->array_listeners[type.val];

        if (listener == listeners[last]) {
            --count;
            return(true);
        }

        for (
            u32 index = 0;
                index < count;
              ++index) {

            if (listener != listeners[index]) {
                continue;
            }

            listeners[index] = listeners[last];
            --count;
            return(true);
        }

        return(false);
    }

    IFB_INTERNAL void
    event_listener_table_execute(
        const event_listener_table* listener_table,
        const event_type_u32        type,
        const event_payload&        payload) {

        assert(listener_table);
        assert(type.val < config::event_type_max);

        const u32 count = listener_table->array_count[type.val];
        assert(count <= config::event_max_listeners_per_event);

        auto listeners = listener_table->array_listeners[type.val];

        for (
            u32 index = 0;
                index < count;
              ++index) {

            listeners[index](payload);
        }
    }
};
