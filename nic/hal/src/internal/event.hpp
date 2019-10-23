//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include "nic/include/base.hpp"
#include "grpc++/grpc++.h"
#include "grpc++/server.h"
#include "gen/proto/event.grpc.pb.h"

using event::EventRequest;
using event::EventResponse;

namespace hal {

#define HAL_MAX_EVENTS              64

hal_ret_t handle_event_request(const EventRequest *req,
                               grpc::ServerWriter<EventResponse> *stream);

}    // namespace hal

#endif    // __EVENT_HPP__

