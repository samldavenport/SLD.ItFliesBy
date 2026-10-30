#pragma once

#include "eng-events.hpp"

namespace ifb {

    struct event_queue {
        u32    count;
        event  events [config::event_queue_capacity];
    };

    IFB_INTERNAL event_queue*
    event_queue_create(
        void) {

        auto queue = (event_queue*)event_mngr_alloc(sizeof(event_queue));
        assert(queue);

        return(queue);
    }

    IFB_INTERNAL void
    event_queue_reset(
        event_queue* q) {

        assert(q);

        q->count = 0;
    }

    IFB_INTERNAL bool
    event_queue_push(
        event_queue*         q,
        const event_type_u32 type,
        const event_payload& payload) {

        assert(q);
        assert(type.val < config::event_type_max);
        assert(q->count <= config::event_queue_capacity);

        u32& index = q->count;

        if (index == config::event_queue_capacity) {
            return(false);
        }

        q->events[index].type    = type;
        q->events[index].payload = payload;
        ++index;
    
        return(true);
    }

    IFB_INTERNAL void
    event_queue_execute_and_reset(
        event_queue*                q,
        const event_listener_table* listener_table) {
       
        assert(q);
        assert(q->count <= config::event_queue_capacity);
        assert(listener_table);

        for (
            u32 i_event = 0;
                i_event < q->count;
              ++i_event) {

            const auto& e = q->events[i_event];
            event_listener_table_execute(listener_table, e.type, e.payload); 
        } 

        q->count = 0;
    }

};
