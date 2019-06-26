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

static bool pd_inited = 0;
int
cli_init (char *ptr)
{
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    capri_cfg_t  capri_cfg;
    catalog      *catalog;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "apollo/capri_p4_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    // initialize PAL
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    catalog = catalog::factory(capri_cfg.cfg_path, "",
                               platform_type_t::PLATFORM_TYPE_HW);
    std::string mpart_json = capri_cfg.cfg_path + "/apollo/" +
        catalog->memory_capacity_str() + "/hbm_mem.json";
    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    // do capri_state_pd_init needed by sdk capri
    // csr init is done inside capri_state_pd_init
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    // do apollo specific initialization
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    pd_inited = 1;
    return 0;
}
