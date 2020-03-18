// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "platform/utils/lif_mgr/lif_mgr.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/hal_cfg.hpp"
#include "platform/capri/capri_common.hpp"
#include "nic/hal/src/internal/cpulif.hpp"
#include "nic/hal/src/internal/cpucb.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace sdk::platform::capri;
using namespace sdk::platform::utils;

namespace hal {

const uint32_t kNumMaxLIFs = 2048;

class lif_mgr *g_lif_manager = nullptr;
program_info *g_pinfo = nullptr;

lif_mgr *
lif_manager()
{
    return g_lif_manager;
}

program_info *
prog_info()
{
    return g_pinfo;
}

namespace nw {

// initialization routine for lif module
extern "C" hal_ret_t
lif_init (hal_cfg_t *hal_cfg)
{
    g_pinfo = program_info::factory((hal_cfg->cfg_path +
                                     "/gen/mpu_prog_info.json").c_str());
    SDK_ASSERT(g_pinfo && hal_cfg->mempartition);

    g_lif_manager = lif_mgr::factory(kNumMaxLIFs, hal_cfg->mempartition, 
                                     "lif2qstate_map");

    // Proxy Init
    if (hal_cfg->features == HAL_FEATURE_SET_IRIS) {
        hal_proxy_svc_init();
    }
    // Moved to customer vrf create
    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_SIM) {
        if (hal_cfg->features == HAL_FEATURE_SET_IRIS) {
            program_cpu_lif();
        }
    }
    return HAL_RET_OK;
}

// cleanup routine for nat module
extern "C" void
lif_exit (void)
{
}

}    // namespace nw
}    // namespace hal
