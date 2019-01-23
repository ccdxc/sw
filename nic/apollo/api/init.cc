/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    init.cc
 *
 * @brief   This file deals with OCI init/teardown API handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/include/api/oci_init.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"
#include "nic/apollo/core/oci_state.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/debug.hpp"

namespace api {

/**
 * @defgroup OCI_INIT_API - init/teardown API handling
 * @ingroup OCI_INIT
 * @{
 */

#define JP4_PRGM        "p4_program"
#define JRXDMA_PRGM     "rxdma_program"
#define JTXDMA_PRGM     "txdma_program"

/**
 * @brief    initialize all common global asic configuration parameters
 * @param[in] asic_cfg   pointer to asic configuration instance
 */
static inline void
asic_global_config_init (asic_cfg_t *asic_cfg)
{
    asic_cfg->asic_type = sdk::platform::asic_type_t::SDK_ASIC_TYPE_CAPRI;
    asic_cfg->cfg_path = g_oci_state.cfg_path();
    asic_cfg->catalog =  g_oci_state.catalogue();
    asic_cfg->mempartition = g_oci_state.mempartition();
    // TODO: @sai please deprecate capri_loader.conf
    asic_cfg->loader_info_file = "capri_loader.conf";
    asic_cfg->default_config_dir = "2x100_hbm";
    asic_cfg->platform = g_oci_state.platform_type();
    asic_cfg->admin_cos = 1;
    asic_cfg->pgm_name = std::string("apollo");
    asic_cfg->completion_func = NULL;
}

/**
 * @brief    initialize all the p4 program specific configuration parameters
 * @param[in] asic_cfg   pointer to asic configuration instance
 */
static inline void
asic_program_config_init (asic_cfg_t *asic_cfg)
{
    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");
}

/**
 * @brief    initialize all the asm specific configuration parameters
 * @param[in] asic_cfg   pointer to asic configuration instance
 */
// TODO: @cruzj, we need to get apollo out of this, so this is generic and
//       can be pushed down to impl_base::init() along with
//       asic_program_config_init(), only asic_global_config_init() should
//       remain here !!!
static inline void
asic_asm_config_init (asic_cfg_t *asic_cfg)
{
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

/**
 * @brief    initialize all the asic configuration parameters
 * @param[in] asic_cfg   pointer to asic configuration instance
 */
static inline void
asic_config_init (asic_cfg_t *asic_cfg)
{
    asic_global_config_init(asic_cfg);
    asic_program_config_init(asic_cfg);
    asic_asm_config_init(asic_cfg);
}

/**
 * @brief    initialize linkmgr to manage ports
 * @param[in] catalog    pointer to asic catalog instance
 * @param[in] cfg_path   path to the global configuration file
 */
static inline sdk_ret_t
linkmgr_init (catalog *catalog, const char *cfg_path)
{
    linkmgr_cfg_t    cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.platform_type = g_oci_state.platform_type();
    cfg.catalog = catalog;
    cfg.cfg_path = cfg_path;
    cfg.port_event_cb = NULL;
    cfg.xcvr_event_cb = NULL;
    cfg.port_log_fn = NULL;

    /**< initialize the linkmgr */
    sdk::linkmgr::linkmgr_init(&cfg);
    /**< start the linkmgr control thread */
    sdk::linkmgr::linkmgr_start();

    return SDK_RET_OK;
}

/**
 * @brief    handle the signal
 * @param[in] sig   signal caught
 * @param[in] info  detailed information about signal
 * @param[in] ptr   pointer to context passed during signal installation, if any
 */
static void
sig_handler (int sig, siginfo_t *info, void *ptr)
{
    OCI_TRACE_DEBUG("Caught signal %d", sig);
    debug::system_dump("hal.info");
}

}    // namespace api

/**
 * @brief        initialize OCI HAL
 * @param[in]    params init time parameters
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_init (oci_init_params_t *params)
{
    sdk_ret_t     ret;
    asic_cfg_t    asic_cfg;

    /**< initializer the logger */
    sdk::lib::logger::init(params->trace_cb);
    register_trace_cb(params->trace_cb);

    api::g_oci_state.set_cfg_path(std::string(std::getenv("HAL_CONFIG_PATH")));
    if (api::g_oci_state.cfg_path().empty()) {
        api::g_oci_state.set_cfg_path(std::string("./"));
    } else {
        api::g_oci_state.set_cfg_path(api::g_oci_state.cfg_path() + "/");
    }
    ret = core::parse_global_config(params->cfg_file, &api::g_oci_state);
    SDK_ASSERT(ret == SDK_RET_OK);
    api::g_oci_state.set_mpartition(sdk::platform::utils::mpartition::factory());
    if ((api::g_oci_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) ||
        (api::g_oci_state.platform_type() == platform_type_t::PLATFORM_TYPE_HAPS) ||
        (api::g_oci_state.platform_type() == platform_type_t::PLATFORM_TYPE_MOCK)) {
        api::g_oci_state.set_catalog(
            catalog::factory(api::g_oci_state.cfg_path() + "catalog_hw.json"));
    } else if (api::g_oci_state.platform_type() == platform_type_t::PLATFORM_TYPE_RTL) {
        api::g_oci_state.set_catalog(
            catalog::factory(api::g_oci_state.cfg_path() + "catalog_rtl.json"));
    } else {
        api::g_oci_state.set_catalog(
            catalog::factory(api::g_oci_state.cfg_path() + "catalog.json"));
    }
    ret = core::parse_pipeline_config("apollo.json", &api::g_oci_state);
    SDK_ASSERT(ret == SDK_RET_OK);

    /**< setup all asic specific config params */
    api::asic_config_init(&asic_cfg);
    SDK_ASSERT(impl_base::init(&asic_cfg) == SDK_RET_OK);
    impl::g_oci_impl_state.init(&api::g_oci_state);

    /**< spin all necessary threads in the system */
    core::thread_spawn(&api::g_oci_state);

    /**< trigger linkmgr initialization */
    api::linkmgr_init(asic_cfg.catalog, asic_cfg.cfg_path.c_str());
    SDK_ASSERT(api::create_ports() == SDK_RET_OK);

    /**< initialize all the signal handlers */
    core::sig_init(SIGUSR1, api::sig_handler);

    return SDK_RET_OK;
}

/**
 * @brief    teardown OCI HAL
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_teardown (void)
{
    // 1. queiesce the chip
    // 2. flush buffers
    // 3. bring links down
    // 4. bring host side down (scheduler etc.)
    // 5. bring asic down (scheduler etc.)
    // 6. kill FTE threads and other other threads
    // 7. flush all logs
    return SDK_RET_OK;
}

/** @} */    // end of OCI_INIT_API
