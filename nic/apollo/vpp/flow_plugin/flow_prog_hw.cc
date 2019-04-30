/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>

#include <nic/sdk/include/sdk/table.hpp>
#include <cstring>
#include <nic/utils/ftl/ftl.hpp>
#include <nic/utils/ftl/ftl_structs.hpp>
#include <nic/sdk/include/sdk/ip.hpp>
#include <nic/sdk/include/sdk/base.hpp>
#include <nic/sdk/include/sdk/types.hpp>
#include <nic/sdk/include/sdk/platform.hpp>
#include <nic/sdk/platform/capri/csrint/csr_init.hpp>
#include <nic/sdk/platform/capri/capri_state.hpp>
#include <nic/sdk/lib/pal/pal.hpp>
#include <nic/sdk/asic/rw/asicrw.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <gen/p4gen/apollo/include/p4pd.h>
#include <nic/p4/common/defines.h>
#include <nic/sdk/platform/capri/capri_p4.hpp>
#include <nic/sdk/asic/pd/pd.hpp>
#include "flow_prog_hw.h"

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

extern "C" {

int
initialize_pds(void)
{
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    capri_cfg_t  capri_cfg;
    sdk_ret_t    ret;

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

    /* initialize PAL */
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    std::string mpart_json = capri_cfg.cfg_path + "/apollo/hbm_mem.json";
    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    /* do capri_state_pd_init needed by sdk capri
     * csr init is done inside capri_state_pd_init */
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    /* do apollo specific initialization */
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    return 0;
}

ftl *
ftl_create(uint32_t table_id,
           uint32_t num_hints,
           uint32_t max_recircs,
           void *key2str,
           void *appdata2str)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.table_id = table_id;
    factory_params.num_hints = num_hints;
    factory_params.max_recircs = max_recircs;
    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);

    return ftl::factory(&factory_params);
}

int
ftl_insert(ftl *obj, ftentry_t *entry, uint32_t hash)
{
    sdk_table_api_params_t params = {0};

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }
    return 0;
}

void
ftl_delete(ftl *obj)
{
    ftl::destroy(obj);
}

}
