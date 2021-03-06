//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// Event segment service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/event.hpp"

Status EventServiceImpl::EventListen(ServerContext* context,
                   grpc::ServerReaderWriter<EventResponse, EventRequest> *stream)
{
    EventRequest    req;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("Handling event operation, event id {}, operation {}, "
                    "stream {}", req.event_id(), req.event_operation(),
                    (void *)stream);

    do {
        while (stream->Read(&req)) {
            ret = hal::handle_event_request(&req, stream);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Event op {} failed for event id {}", req.event_operation(), req.event_id());
                return Status::CANCELLED;
            }
        }

        // if this stream doesn't have any events of interest, we can close
        // this stream
        if (!hal::g_hal_state->event_mgr()->is_listener_active(stream)) {
            HAL_TRACE_DEBUG("Listener {} is not active, removing ...",
                            (void *)stream);
            hal::g_hal_state->event_mgr()->unsubscribe_listener(stream);
            break;
        }
        // This tight loop is taking 100% CPU with lots of sys calls of sched_yield.
        // Commenting this out to reduce CPU utilization from HAL
        usleep(1000000);
        // pthread_yield();
    } while (true);

    return Status::OK;
}
