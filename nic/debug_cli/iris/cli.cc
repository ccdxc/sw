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
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/iris/p4pd_cfg.hpp"
#include "nic/debug_cli/include/cli.hpp"
#include "nic/hal/core/core.hpp"

static bool pd_inited = 0;
int
cli_init (char *ptr)
{
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    p4pd_cfg_t   p4pd_cfg, p4pd_rxdma_cfg, p4pd_txdma_cfg;

    //printf("Initing: Please wait for the prompt ...\n");

    // initialize logger
    cli_logger_init();

    // populate cfg
    pipeline_cfg_init(&p4pd_cfg, &p4pd_rxdma_cfg, &p4pd_txdma_cfg);

    // initialize PAL
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    asic_cfg_t asic_cfg;
    memset(&asic_cfg, 0, sizeof(asic_cfg_t));
    asic_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    // asic_cfg.default_config_dir = std::string(std::getenv("HAL_PBC_INIT_CONFIG"));
    asic_cfg.admin_cos = 1;
    asic_cfg.num_rings = 0;
    asic_cfg.ring_meta = NULL;
    std::string mpart_json = asic_cfg.cfg_path + "/iris/hbm_mem.json";
    asic_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    asic_cfg.repl_entry_width = P4_REPL_ENTRY_WIDTH;
    asic_cfg.pgm_name = std::string("iris");

    asic_cfg.num_pgm_cfgs = 1;
    asic_cfg.pgm_cfg[0].path = std::string("pgm_bin");

    asic_cfg.num_asm_cfgs = 1;
    asic_cfg.asm_cfg[0].name = std::string("iris");
    asic_cfg.asm_cfg[0].path = std::string("asm_bin");
    asic_cfg.asm_cfg[0].symbols_func = NULL;
    asic_cfg.asm_cfg[0].sort_func = NULL;
    asic_cfg.asm_cfg[0].base_addr = std::string(JP4_PRGM);

#if 0
    asic_cfg.asm_cfg[1].name = std::string("p4plus");
    asic_cfg.asm_cfg[1].path = std::string("p4plus_bin");
    asic_cfg.asm_cfg[1].symbols_func = NULL;
    // asic_cfg.asm_cfg[1].symbols_func = hal::pd::common_p4plus_symbols_init;
    asic_cfg.asm_cfg[1].sort_func = NULL;
    asic_cfg.asm_cfg[1].base_addr = std::string(JP4PLUS_PRGM);
#endif

    asic_cfg.catalog = catalog::factory(asic_cfg.cfg_path, "",
                                        platform_type_t::PLATFORM_TYPE_HW);
    auto device_cfg_path  = std::string(SYSCONFIG_PATH) + "/" + DEVICE_CFG_FNAME;
    auto device = sdk::lib::device::factory(device_cfg_path);
    SDK_ASSERT(device != NULL);
    asic_cfg.device_profile = device->device_profile();

    // asic_cfg.completion_func = asiccfg_init_completion_event;
    asic_cfg.completion_func = NULL;
    SDK_ASSERT(sdk::asic::pd::asicpd_init(&asic_cfg) == SDK_RET_OK);

    // do iris specific initialization
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    SDK_ASSERT(sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg) == SDK_RET_OK);

    pd_inited = 1;
    return 0;
}
