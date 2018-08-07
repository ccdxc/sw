// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/dos_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace dos {

DosServiceImpl  g_dos_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "dos" services
    HAL_TRACE_DEBUG("Registering gRPC dos services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_dos_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_dos_svc);
    }
    HAL_TRACE_DEBUG("gRPC dos services registered ...");
    return;
}

// initialization routine for dos module
extern "C" hal_ret_t
doscfg_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for dos module
extern "C" void
doscfg_exit (void)
{
}

}    // namespace l4lb 
}    // namespace hal
