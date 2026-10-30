#ifndef ENG_EVENTS_HPP
#define ENG_EVENTS_HPP

#include "ifb.hpp"
#include "eng-memory.hpp"

namespace ifb {

    struct event;
    struct event_mngr;
    struct event_queue;
    struct event_listener_table;

    IFB_INTERNAL event_mngr*           event_mngr_create             (void);
    IFB_INTERNAL void                  event_mngr_startup            (reservation* res);
    IFB_INTERNAL void                  event_mngr_shutdown           (void);
    IFB_INTERNAL void                  event_mngr_update             (void);
    IFB_INTERNAL void*                 event_mngr_alloc              (const u32 size);
    IFB_INTERNAL bool                  event_mngr_publish            (const event_type_u32 type, const event_payload& payload);
    IFB_INTERNAL bool                  event_mngr_subscribe          (const event_type_u32 type, const fptr_event_listener listener);
    IFB_INTERNAL bool                  event_mngr_unsubscribe        (const event_type_u32 type, const fptr_event_listener listener);

    IFB_INTERNAL event_listener_table* event_listener_table_create   (void); 
    IFB_INTERNAL bool                  event_listener_table_add      (event_listener_table* listener_table, const event_type_u32 type, const fptr_event_listener listener);
    IFB_INTERNAL bool                  event_listener_table_remove   (event_listener_table* listener_table, const event_type_u32 type, const fptr_event_listener listener);
    IFB_INTERNAL void                  event_listener_table_execute  (const event_listener_table* listener_table, const event_type_u32 type, const event_payload& payload);

    IFB_INTERNAL event_queue*          event_queue_create            (void);
    IFB_INTERNAL void                  event_queue_reset             (event_queue* q);
    IFB_INTERNAL bool                  event_queue_push              (event_queue* q, const event_type_u32 type, const event_payload& payload);
    IFB_INTERNAL void                  event_queue_execute_and_reset (event_queue* q, const event_listener_table* listener_table);

    struct event {
        event_type_u32 type;
        event_payload  payload;
    };

};

#endif //ENG_EVENTS_HPP
