//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cerrno>
#include <sys/un.h>
#include "nic/apollo/agent/svc/port.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "gen/proto/debug.pb.h"

#define GRPC_API_PORT    50054

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace core {

std::string g_grpc_server_addr;

static void
svc_reg() {
    ServerBuilder         *server_builder;
    PortSvcImpl           port_svc;
    DeviceSvcImpl         device_svc;
    DebugSvcImpl          debug_svc;

    // do gRPC initialization
    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // register for all the services
    server_builder->RegisterService(&port_svc);
    server_builder->RegisterService(&debug_svc);
    server_builder->RegisterService(&device_svc);
    PDS_TRACE_INFO("gRPC server listening on ... {}",
                   g_grpc_server_addr.c_str());
    core::trace_logger()->flush();
    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
} 

void
grpc_reg_thread_init (void *ctxt)
{
    // wait forver
    svc_reg();
}

void
grpc_reg_thread_exit (void *ctxt) {}

}    // namespace core
