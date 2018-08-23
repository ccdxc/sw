// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/plugins/cfg/lif/lif_manager.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/include/hal_cfg.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {

LIFManager *g_lif_manager = nullptr;

LIFManager *
lif_manager()
{
    return g_lif_manager;
}

namespace nw {

// initialization routine for lif module
extern "C" hal_ret_t
lif_init (hal_cfg_t *hal_cfg)
{
    g_lif_manager = new LIFManager();

    // Allocate LIF 0, so that we don't use it later
    int32_t hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
    HAL_TRACE_DEBUG("Allocated hw_lif_id:{}", hw_lif_id);

    return HAL_RET_OK;
}

// cleanup routine for nat module
extern "C" void
lif_exit (void)
{
}

}    // namespace nw
}    // namespace hal
