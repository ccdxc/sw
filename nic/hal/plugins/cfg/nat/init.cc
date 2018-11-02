// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/nat_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "nic/hal/plugins/cfg/nat/nat.hpp"
#include "nic/utils/nat/addr_db.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace nat {

NatServiceImpl    g_nat_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "nat" services
    HAL_TRACE_DEBUG("Registering gRPC nat services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_nat_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
    }
    HAL_TRACE_DEBUG("gRPC nat services registered ...");
    return;
}

// initialization routine for nat module
extern "C" hal_ret_t
natcfg_init (hal_cfg_t *hal_cfg)
{
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    utils::nat::addr_db_init(HAL_MAX_NAT_ADDR_MAP);
    utils::nat::addr_entry_slab_init();
    return HAL_RET_OK;
}

// cleanup routine for nat module
extern "C" void
natcfg_exit (void)
{
}

}    // namespace nat
}    // namespace hal
