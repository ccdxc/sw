/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    init.cc
 *
 * @brief   This file deals with PDS init/teardown API handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/platform/marvell/marvell.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/lib/kvstore/kvstore.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/upgrade.hpp"
#include "nic/apollo/api/internal/metrics.hpp"
#include "platform/sysmon/sysmon.hpp"
#include "nic/sdk/platform/asicerror/interrupts.hpp"
#include "nic/apollo/api/internal/monitor.hpp"

const pds_obj_key_t k_pds_obj_key_invalid = { 0 };

namespace api {

/**
 * @defgroup PDS_INIT_API - init/teardown API handling
 * @ingroup PDS_INIT
 * @{
 */

/**
 * @brief    initialize all common global asic configuration parameters
 * @param[in] params     initialization time parameters passed by application
 * @param[in] asic_cfg   pointer to asic configuration instance
 */
static inline void
asic_global_config_init (pds_init_params_t *params, asic_cfg_t *asic_cfg)
{
    //asic_cfg->asic_type = api::g_pds_state.catalogue()->asic_type();
    asic_cfg->asic_type = sdk::platform::asic_type_t::SDK_ASIC_TYPE_CAPRI;
    asic_cfg->cfg_path = g_pds_state.cfg_path();
    asic_cfg->catalog =  g_pds_state.catalogue();
    asic_cfg->mempartition = g_pds_state.mempartition();
    asic_cfg->default_config_dir = "2x100_hbm";
    asic_cfg->platform = g_pds_state.platform_type();
    asic_cfg->admin_cos = 1;
    asic_cfg->pgm_name = params->pipeline;
    asic_cfg->completion_func = NULL;
}

/**
 * @brief    initialize linkmgr to manage ports
 * @param[in] catalog    pointer to asic catalog instance
 * @param[in] cfg_path   path to the global configuration file
 */
static inline sdk_ret_t
linkmgr_init (catalog *catalog, const char *cfg_path)
{
    linkmgr_cfg_t cfg;
    marvell_cfg_t marvell_cfg;

    // initialize the marvell switch
    memset(&marvell_cfg, 0, sizeof(marvell_cfg_t));
    marvell_cfg.catalog = catalog;
    sdk::marvell::marvell_switch_init(&marvell_cfg);

    memset(&cfg, 0, sizeof(cfg));
    cfg.platform_type = g_pds_state.platform_type();
    cfg.catalog = catalog;
    cfg.cfg_path = cfg_path;
    cfg.port_event_cb = api::port_event_cb;
    cfg.xcvr_event_cb = api::xcvr_event_cb;
    cfg.port_log_fn = NULL;
    cfg.admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    cfg.mempartition = g_pds_state.mempartition();

    // initialize the linkmgr
    sdk::linkmgr::linkmgr_init(&cfg);
    return SDK_RET_OK;
}

/**
 * @brief    create one uplink per port
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
create_uplinks (void)
{
    sdk_ret_t        ret;
    pds_if_spec_t    spec = { 0 };
    pds_ifindex_t    ifindex, eth_ifindex;

    PDS_TRACE_DEBUG("Creating uplinks ...");
    for (uint32_t port = 1;
        port <= g_pds_state.catalogue()->num_fp_ports(); port++) {
        eth_ifindex = ETH_IFINDEX(g_pds_state.catalogue()->slot(),
                                  port, ETH_IF_DEFAULT_CHILD_PORT);
        ifindex = ETH_IFINDEX_TO_UPLINK_IFINDEX(eth_ifindex);
        spec.key = uuid_from_objid(ifindex);
        spec.type = PDS_IF_TYPE_UPLINK;
        spec.admin_state = PDS_IF_STATE_UP;
        spec.uplink_info.port = uuid_from_objid(eth_ifindex);
        PDS_TRACE_DEBUG("Creating uplink %s", spec.key.str());
        ret = pds_if_create(&spec, PDS_BATCH_CTXT_INVALID);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Uplink if 0x%x creation failed", ifindex);
            break;
        }
    }
    return ret;
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
    PDS_TRACE_DEBUG("Caught signal %d", sig);
}

/**
 * @brief    initialize and start system monitoring
 */
static void
sysmon_init (void)
{
    sysmon_cfg_t sysmon_cfg;
    intr_cfg_t intr_cfg;

    memset(&sysmon_cfg, 0, sizeof(sysmon_cfg_t));
    sysmon_cfg.power_event_cb = power_event_cb;
    sysmon_cfg.temp_event_cb = temperature_event_cb;
    sysmon_cfg.catalog = api::g_pds_state.catalogue();

    // init the sysmon lib
    sysmon_init(&sysmon_cfg);

    intr_cfg.intr_event_cb = interrupt_event_cb;
    // init the interrupts lib
    intr_init(&intr_cfg);

    // schedule sysmon timer
    core::schedule_timers(&api::g_pds_state);
}

static void
system_mac_init (void)
{
    std::string       mac_str;
    mac_addr_t        mac_addr;

    if (api::g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        sdk::platform::readfrukey(BOARD_MACADDRESS_KEY, mac_str);
        mac_str_to_addr((char *)mac_str.c_str(), mac_addr);
        api::g_pds_state.set_system_mac(mac_addr);
    } else {
        // for non h/w platforms, set system MAC to default
        MAC_UINT64_TO_ADDR(mac_addr, PENSANDO_NIC_MAC);
        api::g_pds_state.set_system_mac(mac_addr);
        PDS_TRACE_ERR("system mac 0x%06lx", MAC_TO_UINT64(api::g_pds_state.system_mac()));
    }
}

static std::string
catalog_init (pds_init_params_t *params)
{
    std::string       mem_str;
    uint64_t          datapath_mem;

    // instantiate the catalog
    api::g_pds_state.set_catalog(catalog::factory(
        api::g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW ?
            api::g_pds_state.cfg_path() :
            api::g_pds_state.cfg_path() + params->pipeline,
        "", api::g_pds_state.platform_type()));
    mem_str = api::g_pds_state.catalogue()->memory_capacity_str();
    PDS_TRACE_DEBUG("Memory capacity of the system %s", mem_str.c_str());

    // On Vomero, Uboot gives Linux 6G on boot up, so only 2G is left for Datapath.
    // Load 4G HBM profile on systems with 2G data path memory
    if (api::g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        datapath_mem = pal_mem_get_phys_totalsize();
        PDS_TRACE_DEBUG("Datapath Memory:  %llu(0x%llx) Bytes", datapath_mem, datapath_mem);
        if (datapath_mem == 0x80000000) { //2G
            mem_str = "4g";
            PDS_TRACE_DEBUG("Loading 4G HBM profile");
        }
    }
    return mem_str;
}

static sdk_ret_t
mpartition_init (pds_init_params_t *params, std::string mem_str)
{
    std::string       mem_json;

    // parse hbm memory region configuration file
    if (params->memory_profile == PDS_MEMORY_PROFILE_DEFAULT) {
        mem_json = "hbm_mem.json";
    } else {
        PDS_TRACE_ERR("Unknown profile %u, aborting ...",
                      params->memory_profile);
        return SDK_RET_INVALID_ARG;
    }
    api::g_pds_state.set_mempartition_cfg(mem_json);
    mem_json = api::g_pds_state.cfg_path() + "/" + params->pipeline + "/" +
                   mem_str + "/" + mem_json;

    // check if the memory carving configuration file exists
    if (access(mem_json.c_str(), R_OK) < 0) {
        PDS_TRACE_ERR("memory config file %s doesn't exist or not accessible\n",
                      mem_json.c_str());
        return SDK_RET_INVALID_ARG;
    }
    api::g_pds_state.set_mempartition(
        sdk::platform::utils::mpartition::factory(mem_json.c_str()));
    // below file is used during upgrade
    api::g_pds_state.mempartition()->dump_regions_info(
        api::g_pds_state.cfg_path().c_str());

    PDS_TRACE_DEBUG("Initializing PDS with memory json %s", mem_json.c_str());
    return SDK_RET_OK;
}

static void
spawn_threads (void)
{
    // for Apulu, pciemgr is started as a separate process
    if (g_pds_state.pipeline() != "apulu") {
        // spawn pciemgr thread
        core::spawn_pciemgr_thread(&api::g_pds_state);

        PDS_TRACE_INFO("Waiting for pciemgr server to come up ...");
        // TODO: we need to do better here !! losing 2 seconds
        sleep(2);
    }

    // spawn api thread
    core::spawn_api_thread(&api::g_pds_state);

    // spawn nicmgr thread
    core::spawn_nicmgr_thread(&api::g_pds_state);

    // spawn periodic thread, have to be before linkmgr init
    core::spawn_periodic_thread(&api::g_pds_state);

}

}    // namespace api

std::string
pds_memory_profile_to_string (pds_memory_profile_t profile)
{
    switch (profile) {
    case PDS_MEMORY_PROFILE_DEFAULT:
    default:
        return std::string("");
    }
}

std::string
pds_device_profile_to_string (pds_device_profile_t profile)
{
    switch (profile) {
    case PDS_DEVICE_PROFILE_2PF:
        return std::string("pf2");
    case PDS_DEVICE_PROFILE_3PF:
        return std::string("pf3");
    case PDS_DEVICE_PROFILE_4PF:
        return std::string("pf4");
    case PDS_DEVICE_PROFILE_5PF:
        return std::string("pf5");
    case PDS_DEVICE_PROFILE_6PF:
        return std::string("pf6");
    case PDS_DEVICE_PROFILE_7PF:
        return std::string("pf7");
    case PDS_DEVICE_PROFILE_8PF:
        return std::string("pf8");
    case PDS_DEVICE_PROFILE_DEFAULT:
    default:
        return std::string("");
    }
}

/**
 * @brief        initialize PDS HAL
 * @param[in]    params init time parameters
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_init (pds_init_params_t *params)
{
    sdk_ret_t         ret;
    asic_cfg_t        asic_cfg;
    std::string       mem_str;
    sdk::platform::upg_mode_t upg_mode;

    sdk::lib::device_profile_t device_profile = { 0 };
    device_profile.qos_profile = {9216, 8, 25, 27, 16, 2, {0, 24}};

    sdk::lib::logger::init(params->trace_cb);

    // register trace callback
    register_trace_cb(params->trace_cb);

    // do state initialization
    SDK_ASSERT(api::g_pds_state.init(params->pipeline, params->cfg_file) ==
                   SDK_RET_OK);
    api::g_pds_state.set_event_cb(params->event_cb);

    api::system_mac_init();
    mem_str = api::catalog_init(params);

    // parse pipeline specific configuration
    ret = core::parse_pipeline_config(params->pipeline, &api::g_pds_state);
    SDK_ASSERT(ret == SDK_RET_OK);

    // parse hbm memory region configuration file
    ret = api::mpartition_init(params, mem_str);
    if (ret != SDK_RET_OK) {
        return SDK_RET_ERR;
    }

    api::g_pds_state.set_device_profile(params->device_profile);
    api::g_pds_state.set_memory_profile(params->memory_profile);
    PDS_TRACE_INFO("Initializing PDS with device profile %u, memory profile %u",
                   params->device_profile, params->memory_profile);

    api::g_pds_state.set_memory_profile_string(pds_memory_profile_to_string(params->memory_profile));
    api::g_pds_state.set_device_profile_string(pds_device_profile_to_string(params->device_profile));

    // setup all asic specific config params
    api::asic_global_config_init(params, &asic_cfg);
    asic_cfg.device_profile = &device_profile;

    // skip the threads, ports and monitoring if it is soft initialization
    if (sdk::asic::asic_is_hard_init()) {
        // upgrade init
        ret = api::upg_init(params);
        if (ret != SDK_RET_OK) {
            return SDK_RET_ERR;
        }
        upg_mode = api::g_upg_state->upg_init_mode();
        // set upgrade mode in asic config
        asic_cfg.upg_init_mode = upg_mode;

        // impl init
        SDK_ASSERT(impl_base::init(params, &asic_cfg) == SDK_RET_OK);

        // restore objs in case of upgrade
        ret = api::upg_obj_restore(upg_mode);
        SDK_ASSERT(ret == SDK_RET_OK);

        // spawn threads
        api::spawn_threads();

        // linkmgr init
        api::linkmgr_init(asic_cfg.catalog, asic_cfg.cfg_path.c_str());

        // create ports
        SDK_ASSERT(api::create_ports() == SDK_RET_OK);

        // create uplink interfaces
        while (!api::is_api_thread_ready()) {
            pthread_yield();
        }
        SDK_ASSERT(api::create_uplinks() == SDK_RET_OK);

        // initialize all the signal handlers
        core::sig_init(SIGUSR1, api::sig_handler);

        // temporary workaround - disable sysmon on apulu pipeline to mask
        // learn DPDK corruption
#ifndef APULU
        // initialize and start system monitoring
        api::sysmon_init();
#endif

        // don't interfere with nicmgr
        while (!core::is_nicmgr_ready()) {
            pthread_yield();
        }

        // spin learn thread
        core::spawn_learn_thread(&api::g_pds_state);

        // raise HAL_UP event
        sdk::ipc::broadcast(EVENT_ID_PDS_HAL_UP, NULL, 0);
    } else {
        // impl init
        SDK_ASSERT(impl_base::init(params, &asic_cfg) == SDK_RET_OK);
    }
    return SDK_RET_OK;
}

/**
 * @brief    teardown PDS HAL
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_teardown (void)
{
    // 1. queiesce the chip
    // 2. flush buffers
    // 3. bring links down
    // 4. bring host side down (scheduler etc.)
    // 5. bring asic down (scheduler etc.)
    // 6. kill FTE threads and other other threads
    // 7. flush all logs
    sdk::linkmgr::linkmgr_threads_stop();
    sdk::linkmgr::linkmgr_threads_wait();
    core::threads_stop();
    core::threads_wait();
    if (!sdk::asic::asic_is_soft_init()) {
        impl_base::destroy();
    }
    api::pds_state::destroy(&api::g_pds_state);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_INIT_API
