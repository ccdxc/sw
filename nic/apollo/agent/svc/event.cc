//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/event.hpp"

Status
EventSvcImpl::EventSubscribe(ServerContext* context,
                             grpc::ServerReaderWriter<EventResponse,
                                                      EventRequest> *stream) {
    EventRequest event_req;
    pds_event_spec_t event_spec;

    do {
        while (stream->Read(&event_req)) {
            for (int i = 0; i < event_req.request_size(); i++) {
                PDS_TRACE_DEBUG("Got a event msg from agent, id {}, op {}",
                                event_req.request(i).eventid(),
                                event_req.request(i).action());
                memset(&event_spec, 0, sizeof(event_spec));
                pds_event_spec_proto_to_api_spec(&event_spec,
                                                 event_req.request(i));
                core::handle_event_request(&event_spec, stream);
            }
        }

        // check if listener is still active or not and update listener state
        if (core::update_event_listener(stream) == SDK_RET_ENTRY_NOT_FOUND) {
            // listener unsubscribed to all events, bail out
            break;
        }
        pthread_yield();
    } while (TRUE);
    PDS_TRACE_DEBUG("Closing the listener stream {}", (void *)stream);
    return Status::OK;
}
