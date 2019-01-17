// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/cfg/rdma/rdma.hpp"
#include "gen/hal/svc/rdma_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {

RDMAManager *g_rdma_manager = nullptr;

RDMAManager *
rdma_manager()
{
    return g_rdma_manager;
}

RdmaServiceImpl    g_rdma_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "rdma" services
    HAL_TRACE_DEBUG("Registering gRPC rdma services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS ||
        feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_rdma_svc);
    }
    HAL_TRACE_DEBUG("gRPC rdma services registered ...");
    return;
}

// initialization routine for rdma module
extern "C" hal_ret_t
rdma_init (hal::hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    
    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);
    g_rdma_manager = new RDMAManager();
    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);

    return HAL_RET_OK;
}

// cleanup routine for rdma module
extern "C" void
rdma_exit (void)
{
}

}    // namespace hal
