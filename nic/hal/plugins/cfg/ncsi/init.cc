// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/ncsi_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace ncsi {

NcsiServiceImpl g_ncsi_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    server_builder->RegisterService(&g_ncsi_svc);

    HAL_TRACE_DEBUG("gRPC ncsi services registered ...");
    return;
}

// initialization routine for network module
extern "C" hal_ret_t
ncsi_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret = HAL_RET_OK;

    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);

    return HAL_RET_OK;
}

// cleanup routine for network module
extern "C" void
ncsi_exit (void)
{
}

}    // namespace ncsi
}    // namespace hal
