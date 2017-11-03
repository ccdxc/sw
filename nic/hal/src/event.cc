// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/src/event.hpp"

namespace hal {

void
port_event_create (void *lctxt)
{
#if 0
    EventResponse  evrsp;
    grpc::ServerReaderWriter<EventResponse, EventRequest> *stream;

    stream = (grpc::ServerReaderWriter<EventResponse, EventRequest> *)lctxt;
    evrsp.set_event_id(::event::EVENT_ID_ENDPOINT);
    evrsp.set_api_status(types::API_STATUS_OK);
    evrsp.mutable_ep_event()->set_l2_segment_handle(0x1111);
    evrsp.mutable_ep_event()->set_mac_address(0x0a0a0a0a0a);
    if (!stream->Write(evrsp)) {
        HAL_TRACE_ERR("Event ntfn failed, cleaning up stream state ...");
    }
#endif
}

#if 0
void generate_event (void)
{
    g_hal_state->event_mgr()->walk_listeners(::event::EVENT_ID_PORT, port_event_create);
}
#endif

hal_ret_t
handle_event_request (EventRequest *req,
                      grpc::ServerReaderWriter<EventResponse,
                                               EventRequest> *stream)
{
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("Handling event operation, event id {}, operation {}, stream {}\n",
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
