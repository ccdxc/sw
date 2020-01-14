/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    init.cc
 *
 * @brief   This file deals with PDS init/teardown API handling
 */

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "platform/sysmon/sysmon.hpp"

namespace uuids = boost::uuids;
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
    linkmgr_cfg_t    cfg;

    // program the marvell port
    sdk::lib::pal_program_marvell(MARVELL_PORT_CTRL_REG,
                                  sdk::linkmgr::marvell_port_cfg_1g(),
                                  MARVELL_PORT0);

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
    uuids::uuid      uuid;
    sdk_ret_t        ret;
    pds_if_spec_t    spec = { 0 };
    pds_ifindex_t    ifindex, eth_ifindex;

    PDS_TRACE_DEBUG("Creating uplinks ...");
    for (uint32_t port = 1;
         port <= g_pds_state.catalogue()->num_fp_ports(); port++) {
         eth_ifindex = ETH_IFINDEX(g_pds_state.catalogue()->slot(),
                                   port, ETH_IF_DEFAULT_CHILD_PORT);
        ifindex = ETH_IFINDEX_TO_UPLINK_IFINDEX(eth_ifindex);
        uuid = uuids::random_generator()();
        memcpy(&spec.key, &uuid, PDS_MAX_KEY_LEN);
        spec.key.id[PDS_MAX_KEY_LEN] = '\0';
        spec.type = PDS_IF_TYPE_UPLINK;
        spec.admin_state = PDS_IF_STATE_UP;
        spec.uplink_info.port_num =
            sdk::lib::catalog::ifindex_to_logical_port(eth_ifindex);
        ret = pds_if_create(&spec, PDS_BATCH_CTXT_INVALID);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Uplink if 0x%x creation failed", ifindex);
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

static void
sysmon_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    impl_base::asic_impl()->monitor();
}

static void
power_event_cb (sdk::platform::sensor::system_power_t *power)
{
    PDS_TRACE_VERBOSE("Power of pin is {}W, pout1 is {}W, pout2 is {}W",
                      power->pin/1000000, power->pout1/1000000,
                      power->pout2/1000000);
}

static void
temp_event_cb (sdk::platform::sensor::system_temperature_t *temperature,
               sysmond_hbm_threshold_event_t hbm_event)
{
    PDS_TRACE_VERBOSE("Die temperature is {}C, local temperature is {}C,"
                      " HBM temperature is {}C", temperature->dietemp/1000,
                      temperature->localtemp/1000, temperature->hbmtemp);
}

/**
 * @brief    initialize and start system monitoring
 */
static void
sysmon_init (void)
{
    sysmon_cfg_t sysmon_cfg;

    memset(&sysmon_cfg, 0, sizeof(sysmon_cfg_t));
    sysmon_cfg.power_event_cb = power_event_cb;
    sysmon_cfg.temp_event_cb = temp_event_cb;
    sysmon_cfg.catalog = api::g_pds_state.catalogue();

    // init the sysmon lib
    sysmon_init(&sysmon_cfg);

    // schedule sysmon timer
    core::schedule_timers(&api::g_pds_state, api::sysmon_cb);
}

}    // namespace api

/**
 * @brief        initialize PDS HAL
 * @param[in]    params init time parameters
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_init (pds_init_params_t *params)
{
    sdk_ret_t     ret;
    asic_cfg_t    asic_cfg;
    std::string   mem_json;

    // TODO read from device.conf
    sdk::lib::device_profile_t device_profile = { 0 };
    device_profile.qos_profile = {9216, 8, 25, 27, 16, 2, {0, 24}};

    // initialize the logger
    // TODO fix obfl logger when sdk logger is cleaned up
    sdk::lib::logger::init(params->trace_cb, params->trace_cb);
    register_trace_cb(params->trace_cb);

    // do state initialization
    SDK_ASSERT(api::g_pds_state.init() == SDK_RET_OK);

    if (getenv("VPP_IPC_MOCK_MODE"))
        api::g_pds_state.set_vpp_ipc_mock(true);

    // parse global configuration
    api::g_pds_state.set_cfg_path(std::string(std::getenv("HAL_CONFIG_PATH")));
    if (api::g_pds_state.cfg_path().empty()) {
        api::g_pds_state.set_cfg_path(std::string("./"));
    } else {
        api::g_pds_state.set_cfg_path(api::g_pds_state.cfg_path() + "/");
    }
    ret = core::parse_global_config(params->pipeline, params->cfg_file,
                                    &api::g_pds_state);
    SDK_ASSERT(ret == SDK_RET_OK);

    // instantiate the catalog
    api::g_pds_state.set_catalog(catalog::factory(
        api::g_pds_state.cfg_path(), "", api::g_pds_state.platform_type()));
    PDS_TRACE_DEBUG("Memory capacity of the system %s",
                    api::g_pds_state.catalogue()->memory_capacity_str().c_str());

    // parse pipeline specific configuration
    ret = core::parse_pipeline_config(params->pipeline, &api::g_pds_state);
    SDK_ASSERT(ret == SDK_RET_OK);

    // parse hbm memory region configuration file
    if (params->scale_profile == PDS_SCALE_PROFILE_DEFAULT) {
        mem_json =
            api::g_pds_state.cfg_path() + "/" + params->pipeline + "/" +
                api::g_pds_state.catalogue()->memory_capacity_str() +
                "/hbm_mem.json";
    } else if (params->scale_profile == PDS_SCALE_PROFILE_P1) {
        mem_json =
            api::g_pds_state.cfg_path() + "/" + params->pipeline + "/" +
                api::g_pds_state.catalogue()->memory_capacity_str() +
                "/hbm_mem_p1.json";
    } else if (params->scale_profile == PDS_SCALE_PROFILE_P2) {
        mem_json =
            api::g_pds_state.cfg_path() + "/" + params->pipeline + "/" +
                api::g_pds_state.catalogue()->memory_capacity_str() +
                "/hbm_mem_p2.json";
    } else {
        PDS_TRACE_ERR("Unknown profile %u, aborting ...",
                      params->scale_profile);
        return SDK_RET_INVALID_ARG;
    }
    api::g_pds_state.set_scale_profile(params->scale_profile);
    PDS_TRACE_INFO("Initializing PDS with %s, profile %u",
                   mem_json.c_str(), params->scale_profile);

    // check if the memory carving configuration file exists
    if (access(mem_json.c_str(), R_OK) < 0) {
        PDS_TRACE_ERR("memory config file %s doesn't exist or not accessible\n",
                      mem_json.c_str());
        return ret;
    }
    api::g_pds_state.set_mpartition(
        sdk::platform::utils::mpartition::factory(mem_json.c_str()));

    // setup all asic specific config params
    api::asic_global_config_init(params, &asic_cfg);
    asic_cfg.device_profile = &device_profile;
    SDK_ASSERT(impl_base::init(params, &asic_cfg) == SDK_RET_OK);

    // skip the threads, ports and monitoring if it is soft initialization
    if (sdk::asic::is_soft_init()) {
        return SDK_RET_OK;
    }

    // spawn pciemgr thread.
    core::spawn_pciemgr_thread(&api::g_pds_state);

    PDS_TRACE_INFO("Waiting for pciemgr server to come up ...");
    // TODO: we need to do better here !! losing 2 seconds
    sleep(2);

    // spawn api thread
    core::spawn_api_thread(&api::g_pds_state);

    // spawn nicmgr thread
    core::spawn_nicmgr_thread(&api::g_pds_state);

    // spawn periodic thread, have to be before linkmgr init
    core::spawn_periodic_thread(&api::g_pds_state);

    // create ports
    api::linkmgr_init(asic_cfg.catalog, asic_cfg.cfg_path.c_str());
    SDK_ASSERT(api::create_ports() == SDK_RET_OK);

    // create uplink interfaces
    while (!api::is_api_thread_ready()) {
        pthread_yield();
    }
    SDK_ASSERT(api::create_uplinks() == SDK_RET_OK);

    // initialize all the signal handlers
    core::sig_init(SIGUSR1, api::sig_handler);

    // initialize and start system monitoring
    api::sysmon_init();

    // don't interfere with nicmgr
    while (!core::is_nicmgr_ready()) {
         pthread_yield();
    }

    // spin learn thread
    core::spawn_learn_thread(&api::g_pds_state);

    // spin fte thread for Athena
    core::spawn_fte_thread(&api::g_pds_state);

    // raise HAL_UP event
    sdk::ipc::broadcast(EVENT_ID_PDS_HAL_UP, NULL, 0);

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
    core::threads_stop();
    if (!sdk::asic::is_soft_init()) {
        impl_base::destroy();
    }
    api::pds_state::destroy(&api::g_pds_state);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_INIT_API
