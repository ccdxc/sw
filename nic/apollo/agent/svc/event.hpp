// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_EVENT_SVC_HPP__
#define __AGENT_EVENT_SVC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/event.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::EventSvc;
using pds::EventRequest_EventSpec;
using pds::EventRequest;
using pds::EventResponse;

class EventSvcImpl final : public EventSvc::Service {
public:
    Status EventSubscribe(ServerContext *context,
                          grpc::ServerReaderWriter<EventResponse,
                          EventRequest> *stream) override;
};

#endif    // __AGENT_EVENT_SVC_HPP__
