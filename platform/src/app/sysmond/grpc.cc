/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <grpc++/grpc++.h>
#include "platform/src/app/sysmond/logger.h"
#include "platform/src/app/sysmond/sysmond_cb.hpp"

using grpc::Status;
using std::chrono::seconds;

std::shared_ptr<grpc::Channel> g_channel;
std::unique_ptr<Port::Stub> g_port_stub;

#define HAL_GRPC_API_TIMEOUT 25             /* Secs */

#define SET_TIMEOUT()                                                       \
    uint8_t timeout = HAL_GRPC_API_TIMEOUT;                                 \
    std::chrono::system_clock::time_point deadline =                        \
        std::chrono::system_clock::now() + seconds(timeout);                \
    context.set_deadline(deadline);

bool
port_handle_api_status(types::ApiStatus api_status,
                            uint32_t port_id) {
    switch(api_status) {
        case types::API_STATUS_OK:
            return true;

        case types::API_STATUS_NOT_FOUND:
           TRACE_INFO(GetLogger(), "Port {} not found", port_id);
            return false;

        case types::API_STATUS_EXISTS_ALREADY:
           TRACE_INFO(GetLogger(), "Port {} exists already", port_id);
            return false;

        default:
            assert(0);
    }

    return true;
}

int
port_get (uint32_t port_id, port::PortOperState *port_status) {

    port::PortGetRequest      *req;
    port::PortGetRequestMsg   req_msg;
    port::PortGetResponseMsg  rsp_msg;
    grpc::ClientContext       context;
    grpc::Status              status;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_port_id(port_id);

    SET_TIMEOUT();
    status = g_port_stub->PortGet(&context, req_msg, &rsp_msg);

    if (status.ok()) {
        if (port_handle_api_status(
            rsp_msg.response(0).api_status(), port_id) == true) {
            *port_status = rsp_msg.response(0).status().link_status().oper_state();
        } else {
            return -1;
        }
    }
            return 0;
}

sdk_ret_t
connect_hal(void)
{
    grpc::ChannelArguments channel_args;
    std::string svc_url;

    if (getenv("HAL_SOCK_PATH")) {
        svc_url = std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock";
    } else if (getenv("HAL_GRPC_PORT")) {
        svc_url = std::string("localhost:") + getenv("HAL_GRPC_PORT");
    } else {
        svc_url = std::string("localhost:50054");
    }

    channel_args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 100); // Time bet 1st & 2nd attempts
    channel_args.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, 100); // Min time bet subseq. attemps;
    g_channel = grpc::CreateCustomChannel(svc_url, grpc::InsecureChannelCredentials(),
                                        channel_args);

   TRACE_INFO(GetLogger(), "Connecting to HAL at: {}", svc_url.c_str());
    auto state = g_channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        g_channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = g_channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << g_channel->GetState(true) << endl;
    }
   TRACE_INFO(GetLogger(), "Connected to HAL at: {}", svc_url.c_str());
    g_port_stub = Port::NewStub(g_channel);

    return SDK_RET_OK;
}

void
sysmon_grpc_init(void) {
    connect_hal();
}
