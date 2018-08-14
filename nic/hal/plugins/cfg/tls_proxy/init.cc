// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/tls_proxy_cb2_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace tls_proxy_cb {

TlsProxyCbServiceImpl    g_tls_proxy_cb_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "tls_proxy_cb" services
    HAL_TRACE_DEBUG("Registering gRPC tls_proxy_cb services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_tls_proxy_cb_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC tls_proxy_cb services registered ...");
    return;
}

// initialization routine for tls_proxy_cb module
extern "C" hal_ret_t
tls_proxy_cb_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for tls_proxy_cb module
extern "C" void
tls_proxy_cb_exit (void)
{
}

}    // namespace tls_proxy_cb 
}    // namespace hal
