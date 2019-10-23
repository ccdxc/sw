//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/src/internal/event.hpp"

namespace hal {

#if 0
// TEST CODE - BEGIN
bool
port_event_create (utils::event_id_t event_id, void *event_ctxt, void *lctxt)
{
    EventResponse  evrsp;
    grpc::ServerReaderWriter<EventResponse, EventRequest> *stream;

    HAL_TRACE_DEBUG("Generating event {} to listener {}",
                    event_id, lctxt);
    stream = (grpc::ServerReaderWriter<EventResponse, EventRequest> *)lctxt;
    evrsp.set_event_id(static_cast<::event::EventId>(event_id));
    evrsp.set_api_status(types::API_STATUS_OK);
    evrsp.mutable_ep_event()->set_l2_segment_handle(0x1111);
    evrsp.mutable_ep_event()->set_mac_address(0x0a0a0a0a0a);
    if (!stream->Write(evrsp)) {
        HAL_TRACE_ERR("Event ntfn failed");
        return false;     // indicates that stream should be closed
    }

    return true;
}

void generate_event (void)
{
    HAL_TRACE_DEBUG("Generating event {} to all listeners",
                    ::event::EVENT_ID_ENDPOINT);
    g_hal_state->event_mgr()->walk_listeners(::event::EVENT_ID_ENDPOINT, NULL,
                                             port_event_create);
}
// TEST CODE - END
#endif

hal_ret_t
handle_event_request (const EventRequest *req,
                      grpc::ServerWriter<EventResponse> *stream)
{
    hal_ret_t    ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Handling event operation, event id {}, operation {}, stream {}",
                    req->event_id(), req->event_operation(), (void *)stream);
    if (req->event_operation() == ::event::EVENT_OP_SUBSCRIBE) {
        ret = g_hal_state->event_mgr()->subscribe(static_cast<utils::event_id_t>(req->event_id()),
                                                  stream);
    } else if (req->event_operation() == ::event::EVENT_OP_UNSUBSCRIBE) {
        ret = g_hal_state->event_mgr()->unsubscribe(static_cast<utils::event_id_t>(req->event_id()),
                                                    stream);
    }

    return ret;
}

}    // namespace hal
