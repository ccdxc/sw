//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/hal.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "gen/hal/svc/gft_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void
svc_reg (hal::hal_cfg_t *hal_cfg)
{
    ServerBuilder            *server_builder = (ServerBuilder *)hal_cfg->server_builder;
    GftServiceImpl           gft_svc;
    SystemServiceImpl        system_svc;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");
    // register all services
    server_builder->RegisterService(&system_svc);
    //server_builder->RegisterService(&gft_svc);

    HAL_TRACE_INFO("gRPC server listening on ... {}",
                   g_grpc_server_addr.c_str());
    hal::utils::hal_logger()->flush();
    HAL_SYSLOG_INFO("HAL-STATUS:UP");

    // inform hal status
    hal::svc_init_done();

    // assemble the server
    std::unique_ptr<Server> server(server_builder->BuildAndStart());

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}
