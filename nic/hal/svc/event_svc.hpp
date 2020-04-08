// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __EVENT_SVC_HPP__
#define __EVENT_SVC_HPP__

#include "nic/include/base.hpp"
#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/event.grpc.pb.h"

using event::Event;
using event::EventRequest;
using event::EventResponse;

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerWriter;

class EventServiceImpl final : public Event::Service {
public:
    Status EventListen(ServerContext* context,
                       grpc::ServerReaderWriter<EventResponse, EventRequest>* writer) override;
};

#endif    // __EVENT_SVC_HPP__

