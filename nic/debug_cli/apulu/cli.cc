//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/sdk/asic/cmn/asic_state.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/impl/apulu/impl_utils.hpp"
#include "nic/debug_cli/include/cli.hpp"

using namespace sdk::asic::pd;

int
cli_init (char *ptr)
{
    pal_ret_t    pal_ret;
    asic_cfg_t  asic_cfg;
    catalog      *catalog;
    p4pd_cfg_t   p4pd_cfg;

    // initialize PAL
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    cli_logger_init();

    memset(&asic_cfg, 0, sizeof(asic_cfg_t));
    asic_cfg.cfg_path = std::string(std::getenv("CONFIG_PATH"));
    catalog = catalog::factory(asic_cfg.cfg_path, "",
                               platform_type_t::PLATFORM_TYPE_HW);
    std::string mpart_json =
        asic_cfg.cfg_path + "/apulu/" + catalog->memory_capacity_str() +
            "/hbm_mem.json";
    asic_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    SDK_ASSERT(sdk::asic::asic_state_init(&asic_cfg) == SDK_RET_OK);
    // do capri_state_pd_init needed by sdk capri
    // csr init is done inside capri_state_pd_init
    SDK_ASSERT(asicpd_state_pd_init(&asic_cfg) == SDK_RET_OK);

    memset(&p4pd_cfg, 0, sizeof(p4pd_cfg_t));
    p4pd_cfg.cfg_path = std::getenv("CONFIG_PATH");
    api::impl::pipeline_p4_hbm_init(&p4pd_cfg, false);
    return 0;
}
