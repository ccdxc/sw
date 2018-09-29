// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/ipsec_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace ipsec {

IpsecServiceImpl    g_ipsec_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "ipsec" services
    HAL_TRACE_DEBUG("Registering gRPC ipsec services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_ipsec_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC ipsec services registered ...");
    return;
}

// initialization routine for ipsec module
extern "C" hal_ret_t
ipsec_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for ipsec module
extern "C" void
ipsec_exit (void)
{
}

}    // namespace telemetry
}    // namespace hal
