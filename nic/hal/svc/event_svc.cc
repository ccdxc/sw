//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// Event segment service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/event.hpp"

Status EventServiceImpl::EventListen(ServerContext* context, const EventRequest* req, 
                   ServerWriter<EventResponse>* stream)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Handling event operation, event id {}, operation {}, stream {}",
                    req->event_id(), req->event_operation(), (void *)stream);

    ret = hal::handle_event_request(req, stream);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Event op {} failed for event id {}", req->event_operation(), req->event_id());
        return Status::CANCELLED;
    }

    return Status::OK;
}
