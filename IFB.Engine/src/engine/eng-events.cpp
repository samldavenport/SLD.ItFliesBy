#pragma once

#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "eng-events.hpp"

namespace ifb {

    IFB_ENGINE_API bool
    eng_event_listener_subscribe(
        const event_type_u32      type,
        const fptr_event_listener listener) {

        const bool did_subscribe = event_mngr_subscribe(type, listener);
        return(did_subscribe);
    } 

    IFB_ENGINE_API bool
    eng_event_listener_unsubscribe(
        const event_type_u32      type,
        const fptr_event_listener listener) {

        const bool did_unsubscribe = event_mngr_unsubscribe(type, listener);
        return(did_unsubscribe);
    } 
};
