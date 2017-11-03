//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// Event segment service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/src/event.hpp"

Status
EventServiceImpl::EventListen(ServerContext* context,
                              grpc::ServerReaderWriter<EventResponse,
                                                       EventRequest> *stream)
{
    EventRequest    req;

    do {
        while (stream->Read(&req)) {
            HAL_TRACE_DEBUG("Processing event request");
            hal::handle_event_request(&req, stream);
        }
        // TODO: if this stream has no events of interest, we can return from
        // this function
        if (false) {
        } else {
            pthread_yield();
        }
    } while (TRUE);
    
    return Status::OK;
}
