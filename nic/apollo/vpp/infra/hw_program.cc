/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>
#include <cstring>

#include <nic/sdk/include/sdk/table.hpp>
#include <nic/sdk/platform/capri/csrint/csr_init.hpp>
#include <nic/sdk/platform/capri/capri_state.hpp>
#include <nic/sdk/lib/pal/pal.hpp>
#include <nic/sdk/asic/rw/asicrw.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/p4/common/defines.h>
#include <nic/sdk/platform/capri/capri_p4.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <nic/sdk/asic/pd/pd.hpp>
#include "hw_program.h"
#include "pd_utils.h"

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

int
initialize_pds(void)
{
    /* Todo: Hanlde init function ordering across plugins */
#if 0
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    capri_cfg_t  capri_cfg;
    sdk_ret_t    ret;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_p4_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

#if 0
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = PDS_PLATFORM "/capri_txdma_table_map.json",
        .p4pd_pgm_name       = PDS_PLATFORM "_p4",
        .p4pd_rxdma_pgm_name = PDS_PLATFORM "_rxdma",
        .p4pd_txdma_pgm_name = PDS_PLATFORM "_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
#endif
    platform_type_t platform_type = pds_get_platform_type();

    /* initialize PAL */
    pal_ret = sdk::lib::pal_init(platform_type);

    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    catalog *platform_catalog =  catalog::factory(capri_cfg.cfg_path,
                                                  "",
                                                  platform_type);
    std::string mpart_json = capri_cfg.cfg_path + "/" +
                             PDS_PLATFORM + "/" +
                             platform_catalog->memory_capacity_str() +
                             "/hbm_mem.json";

    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    /* do capri_state_pd_init needed by sdk capri
     * csr init is done inside capri_state_pd_init */
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    /* do apollo specific initialization */
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

#if 0
    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);
#endif
    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    capri::capri_table_csr_cache_inval_init();
#endif
    return 0;
}

}
