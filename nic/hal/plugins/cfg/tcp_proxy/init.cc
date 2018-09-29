// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/tcp_proxy_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace tcp_proxy {

TcpProxyServiceImpl    g_tcp_proxy_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "tcp_proxy" services
    HAL_TRACE_DEBUG("Registering gRPC tcp_proxy services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_tcp_proxy_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC tcp_proxy services registered ...");
    return;
}

// initialization routine for tcp_proxy module
extern "C" hal_ret_t
tcp_proxy_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for tcp_proxy module
extern "C" void
tcp_proxy_exit (void)
{
}

}    // namespace tcp_proxy 
}    // namespace hal
