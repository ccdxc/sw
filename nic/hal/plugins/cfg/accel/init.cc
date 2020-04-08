// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/hal/svc/accel_rgroup_svc.hpp"
#include "nic/hal/plugins/cfg/accel/accel_rgroup.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace accel {

AccelRGroupServiceImpl    g_accel_rgroup_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "accel" services
    HAL_TRACE_DEBUG("Registering gRPC accel services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_accel_rgroup_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC accel services registered ...");
    return;
}

// initialization routine for accel module
extern "C" hal_ret_t
accelcfg_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for accel module
extern "C" void
accelcfg_exit (void)
{
}

extern "C" void
accelcfg_thread_init(int tid)
{
    HAL_ABORT(hal::accel_rgroup_init(tid, ACCEL_RING_ID_MAX) == HAL_RET_OK);
}

extern "C" void
accelcfg_thread_exit(int tid)
{
    hal::accel_rgroup_fini(tid);
}

}    // namespace accel
}    // namespace hal
