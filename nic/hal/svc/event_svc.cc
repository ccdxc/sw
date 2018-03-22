//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// Event segment service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/internal/event.hpp"

Status
EventServiceImpl::EventListen(ServerContext* context,
                              grpc::ServerReaderWriter<EventResponse,
                                                       EventRequest> *stream)
{
    EventRequest    req;

    do {
        while (stream->Read(&req)) {
            hal::handle_event_request(&req, stream);
        }

        // if this stream doesn't have any events of interest, we can close
        // this stream
        if (!hal::g_hal_state->event_mgr()->is_listener_active(stream)) {
            HAL_TRACE_DEBUG("Listener {} is not active, removing ...",
                            (void *)stream);
            hal::g_hal_state->event_mgr()->unsubscribe_listener(stream);
            break;
        }
        pthread_yield();
    } while (TRUE);
    HAL_TRACE_DEBUG("Closing the listener stream {}", (void *)stream);

    return Status::OK;
}
