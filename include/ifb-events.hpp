#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "ifb.hpp"

namespace ifb {

    struct event_mngr;

    IFB_INTERNAL event_mngr* event_mngr_create  (void);
    IFB_INTERNAL void        event_mngr_startup (void);

};

#endif //EVENTS_HPP
