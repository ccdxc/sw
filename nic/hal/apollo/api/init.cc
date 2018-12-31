/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    init.cc
 *
 * @brief   This file deals with OCI init/teardown API handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/hal/apollo/include/api/oci_init.hpp"
#include "nic/hal/apollo/framework/asic_impl_base.hpp"

/**
 * @defgroup OCI_VCN_API - batch API handling
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief        initialize OCI HAL
 * @param[in]    params init time parameters
 * @return #SDK_RET_OK on success, failure status code on error
 */

// TODO: will have to parse hal.json and apollo.json here and fill
//       all params !! top level function providing this kind of
//       detail looks odd, some of this has to be NCC generated ?
#define JP4_PRGM        "p4_program"
#define JRXDMA_PRGM     "rxdma_program"
#define JTXDMA_PRGM     "txdma_program"
static void
fill_asic_cfg (asic_cfg_t *asic_cfg) {
    asic_cfg->cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    if (asic_cfg->cfg_path.empty()) {
        asic_cfg->cfg_path = std::string("./");
    } else {
        asic_cfg->cfg_path += "/";
    }
    //catalog = sdk::lib::catalog::factory(asic_cfg->cfg_path + "catalog.json");
    asic_cfg->loader_info_file = "capri_loader.conf";
    asic_cfg->default_config_dir = "2x100_hbm";
    asic_cfg->platform = platform_type_t::PLATFORM_TYPE_SIM;
    asic_cfg->admin_cos = 1;
    asic_cfg->pgm_name = std::string("apollo");

    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");

    asic_cfg->num_asm_cfgs = 3;
    memset(asic_cfg->asm_cfg, 0, sizeof(asic_cfg->asm_cfg));
    asic_cfg->asm_cfg[0].name = std::string("apollo_p4");
    asic_cfg->asm_cfg[0].path = std::string("p4_asm");
    asic_cfg->asm_cfg[0].base_addr = std::string(JP4_PRGM);
    // TODO: this doesn't seem to fit any where, may be
    //       just sort_mpu_programs should be in apollo_impl class
    //asic_cfg->asm_cfg[0].sort_func = sort_mpu_programs;
    asic_cfg->asm_cfg[1].name = std::string("apollo_rxdma");
    asic_cfg->asm_cfg[1].path = std::string("rxdma_asm");
    asic_cfg->asm_cfg[1].base_addr = std::string(JRXDMA_PRGM);
    asic_cfg->asm_cfg[2].name = std::string("apollo_txdma");
    asic_cfg->asm_cfg[2].path = std::string("txdma_asm");
    asic_cfg->asm_cfg[2].base_addr = std::string(JTXDMA_PRGM);
}

// TODO:
// 1. linkmgr_init()
// 2. periodic_thread_init()
// 3. etc.
sdk_ret_t
oci_init (oci_init_params_t *params)
{
    asic_cfg_t        asic_cfg;
    asic_impl_base    *asic_impl;

    /**< initializer the logger */
    sdk::lib::logger::init(params->debug_trace_cb, params->error_trace_cb);

    // TODO: setup all asic specific config params
    fill_asic_cfg(&asic_cfg);
    asic_impl = asic_impl_base::factory(&asic_cfg);
    SDK_ASSERT(asic_impl != NULL);
    asic_impl->asic_init();

    return sdk::SDK_RET_OK;
}

/**
 * @brief    teardown OCI HAL
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_teardown(void)
{
    // 1. queiesce the chip
    // 2. flush buffers
    // 3. bring links down
    // 4. bring host side down (scheduler etc.)
    // 5. bring asic down (scheduler etc.)
    // 6. kill FTE threads and other other threads
    // 7. flush all logs
    return sdk::SDK_RET_OK;
}

/** @} */    // end of OCI_VCN_API
