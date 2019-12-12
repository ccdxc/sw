//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/platform/capri/csrint/csr_init.hpp"
#include "nic/sdk/platform/capri/capri_state.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/impl/poseidon/impl_utils.hpp"
#include "nic/debug_cli/include/cli.hpp"

static bool pd_inited = 0;
int
cli_init (char *ptr)
{
    pal_ret_t    pal_ret;
    capri_cfg_t  capri_cfg;
    catalog      *catalog;
    p4pd_cfg_t   p4pd_cfg;

    // initialize PAL
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    cli_logger_init();

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    catalog = catalog::factory(capri_cfg.cfg_path, "",
                               platform_type_t::PLATFORM_TYPE_HW);
    std::string mpart_json = capri_cfg.cfg_path + "/poseidon/" +
        catalog->memory_capacity_str() + "/hbm_mem.json";
    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    // do capri_state_pd_init needed by sdk capri
    // csr init is done inside capri_state_pd_init
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    api::impl::pipeline_p4_hbm_init(&p4pd_cfg, false);
    pd_inited = 1;
    return 0;
}
