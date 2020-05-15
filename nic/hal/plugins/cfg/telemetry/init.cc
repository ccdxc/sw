// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/svc/telemetry_svc.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace telemetry {

TelemetryServiceImpl    g_telemetry_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "telemetry" services
    HAL_TRACE_DEBUG("Registering gRPC telemetry services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_telemetry_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC telemetry services registered ...");
    return;
}

// initialization routine for telemetry module
extern "C" hal_ret_t
telemetrycfg_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for mcast module
extern "C" void
telemetrycfg_exit (void)
{
}

}    // namespace telemetry
}    // namespace hal
