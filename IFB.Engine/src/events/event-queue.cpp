#pragma once

#include "eng-events.hpp"
#include "ifb-platform.hpp"

namespace ifb {

    // any thread can publish, the listeners are only called on the main thread
    struct event_queue {
        pfm_thread_mutex lock;
        u32              count;
        event            events [config::event_queue_capacity];
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

        pfm_thread_mutex_lock(q->lock);
        q->count = 0;
        pfm_thread_mutex_unlock(q->lock);
    }

    IFB_INTERNAL bool
    event_queue_push(
        event_queue*         q,
        const event_type_u32 type,
        const event_payload& payload) {

        assert(q);
        assert(type.val < config::event_type_max);

        pfm_thread_mutex_lock(q->lock);
        assert(q->count <= config::event_queue_capacity);

        u32& index = q->count;

        if (index == config::event_queue_capacity) {
            pfm_thread_mutex_unlock(q->lock);
            return(false);
        }

        q->events[index].type    = type;
        q->events[index].payload = payload;
        ++index;

        pfm_thread_mutex_unlock(q->lock);
        return(true);
    }

    IFB_INTERNAL void
    event_queue_execute_and_reset(
        event_queue*                q,
        const event_listener_table* listener_table) {
       
        assert(q);
        assert(listener_table);

        // each event is copied out, so nothing is locked while the listeners run
        // an event published during this runs in this pass too
        u32 i_event = 0;
        for (;;) {

            pfm_thread_mutex_lock(q->lock);
            assert(q->count <= config::event_queue_capacity);

            if (i_event >= q->count) {
                q->count = 0;
                pfm_thread_mutex_unlock(q->lock);
                break;
            }

            const event e = q->events[i_event];
            ++i_event;

            pfm_thread_mutex_unlock(q->lock);

            event_listener_table_execute(listener_table, e.type, e.payload);
        }
    }

};
