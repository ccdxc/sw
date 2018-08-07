// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/l4lb_svc_gen.hpp"
#include "nic/hal/svc/session_svc.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace l4lb {

L4LbServiceImpl  g_l4lb_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "l4lb" services
    HAL_TRACE_DEBUG("Registering gRPC l4lb services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_l4lb_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC l4lb services registered ...");
    return;
}

// initialization routine for l4lb module
extern "C" hal_ret_t
l4lbcfg_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for l4lb module
extern "C" void
l4lbcfg_exit (void)
{
}

}    // namespace l4lb 
}    // namespace hal
