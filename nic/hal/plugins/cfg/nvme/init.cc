// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/p4/common/defines.h"
#include "gen/hal/svc/nvme_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {

NVMEManager *g_nvme_manager = nullptr;

NVMEManager *
nvme_manager()
{
    return g_nvme_manager;
}

NvmeServiceImpl    g_nvme_svc;

void
src_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "nvme" services
    HAL_TRACE_DEBUG("Registering gRPC nvme services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS ||
        feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_nvme_svc);
    }
    HAL_TRACE_DEBUG("gRPC nvme services registered ...");
    return;
}

// initialization routine for nvme module
extern "C" hal_ret_t
nvme_init (hal::hal_cfg_t *hal_cfg)
{
    src_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);

    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);
    g_nvme_manager = new NVMEManager();
    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);

    return HAL_RET_OK;
}

// cleanup routine for nvme module
extern "C" void
nvme_exit (void)
{
}

}    // namespace hal
