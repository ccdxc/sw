// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/gft_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace gft {

GftServiceImpl    g_gft_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "gft" services
    HAL_TRACE_DEBUG("Registering gRPC gft services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_gft_svc);
    }
    HAL_TRACE_DEBUG("gRPC gft services registered ...");
    return;
}

// initialization routine for gft module
extern "C" hal_ret_t
gft_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for gft module
extern "C" void
gft_exit (void)
{
}

}    // namespace gft
}    // namespace hal
