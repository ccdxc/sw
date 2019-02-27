/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    core.cc
 *
 * @brief   This file contains core helper functions
 */

#include <string>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/periodic/periodic.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"

using boost::property_tree::ptree;

#define SESSION_AGE_SCAN_INTVL          1     // in seconds
#define SYSTEM_SCAN_INTVL               10    // in seconds

// TODO: create a "system" class and move all this into that class
namespace core {

thread *g_thread_store[THREAD_ID_MAX];

static sdk_ret_t
parse_cores_config (ptree &pt, pds_state *state)
{
    state->set_control_cores_mask(
        std::stoul(pt.get<string>("control_cores_mask"), nullptr, 16));
    state->set_num_control_cores(
        sdk::lib::count_bits_set(state->control_cores_mask()));
    state->set_data_cores_mask(
        std::stoul(pt.get<string>("data_cores_mask"), nullptr, 16));
    state->set_num_data_cores(
        sdk::lib::count_bits_set(state->data_cores_mask()));

    /**
     * update thread library so it knows which threads to pin to
     * which cores
     */
    sdk::lib::thread::control_cores_mask_set(state->control_cores_mask());
    sdk::lib::thread::data_cores_mask_set(state->data_cores_mask());

    return SDK_RET_OK;
}

sdk_ret_t
parse_pipeline_config (string pipeline, pds_state *state)
{
    ptree     pt;
    string    cfg_file;

    cfg_file = state->cfg_path() + pipeline + "/" + pipeline + ".json";

    /**< make sure cfg file exists */
    if (access(cfg_file.c_str(), R_OK) < 0) {
        PDS_TRACE_ERR("config file %s doesn't exist or not accessible\n",
                      cfg_file.c_str());
        return SDK_RET_ERR;
    }

    /**< parse the config now */
    std::ifstream json_cfg(cfg_file.c_str());
    read_json(json_cfg, pt);
    parse_cores_config(pt, state);
    return SDK_RET_OK;
}

sdk_ret_t
parse_global_config (string pipeline, string cfg_file, pds_state *state)
{
    ptree     pt;

    cfg_file = state->cfg_path() + pipeline + "/" + cfg_file;

    /**< make sure global config file exists */
    if (access(cfg_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                cfg_file.c_str());
        return SDK_RET_ERR;
    }

    /**< parse the config now */
    std::ifstream json_cfg(cfg_file.c_str());
    read_json(json_cfg, pt);
    try {
        std::string mode = pt.get<std::string>("mode");
        if (mode == "sim") {
            state->set_platform_type(platform_type_t::PLATFORM_TYPE_SIM);
        } else if (mode == "hw") {
            state->set_platform_type(platform_type_t::PLATFORM_TYPE_HW);
        } else if (mode == "rtl") {
            state->set_platform_type(platform_type_t::PLATFORM_TYPE_RTL);
        } else if (mode == "haps") {
            state->set_platform_type(platform_type_t::PLATFORM_TYPE_HAPS);
        } else if (mode == "mock") {
            state->set_platform_type(platform_type_t::PLATFORM_TYPE_MOCK);
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return sdk::SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static void
session_age_cb (void *timer, uint32_t timer_id, void *ctxt)
{
}

static void
sysmon_cb (void *timer, uint32_t timer_id, void *ctxt)
{
}

sdk_ret_t
schedule_timers (void)
{
    void    *aging_timer, *sysmon_timer;

    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // start periodic timer for aging sessions
    aging_timer = sdk::lib::timer_schedule(
                      TIMER_ID_SESSION_AGE,
                      SESSION_AGE_SCAN_INTVL * TIME_MSECS_PER_SEC,
                      nullptr, session_age_cb, true);
    if (aging_timer == NULL) {
        PDS_TRACE_ERR("Failed to start session aging timer\n");
        return SDK_RET_ERR;
    }
    PDS_TRACE_DEBUG("Started periodic session aging timer with %us intvl",
                    SESSION_AGE_SCAN_INTVL);

    // start periodic timer for scanning system interrupts, temparature, power
    // etc.
    sysmon_timer = sdk::lib::timer_schedule(
                       TIMER_ID_SYSTEM_SCAN,
                       SYSTEM_SCAN_INTVL * TIME_MSECS_PER_SEC,
                       nullptr, sysmon_cb, true);
    if (sysmon_timer == NULL) {
        PDS_TRACE_ERR("Failed to start system monitoring timer\n");
        return SDK_RET_ERR;
    }
    PDS_TRACE_DEBUG("Started periodic system scan timer with %us intvl",
                    SYSTEM_SCAN_INTVL);

    return SDK_RET_OK;
}

/**
 * starting point for the periodic thread loop
 */
static void *
periodic_thread_start (void *ctxt)
{
    /**< initialize timer wheel */
    sdk::lib::periodic_thread_init(ctxt);
    /**< run main loop */
    sdk::lib::periodic_thread_run(ctxt);

    return NULL;
}

/**
 * wrapper API to create all threads
 */
static sdk::lib::thread *
thread_create (const char *name, uint32_t thread_id,
               sdk::lib::thread_role_t thread_role,
               uint64_t cores_mask,
               sdk::lib::thread_entry_func_t entry_func,
               uint32_t thread_prio, int sched_policy, void *data)
{
    g_thread_store[thread_id] =
        sdk::lib::thread::factory(name, thread_id, thread_role, cores_mask,
                                  entry_func, thread_prio, sched_policy,
                                  (thread_role == sdk::lib::THREAD_ROLE_DATA) ?
                                       false : true);
    if (g_thread_store[thread_id]) {
        g_thread_store[thread_id]->set_data(data);
    }

    return g_thread_store[thread_id];
}

/**
 * spawn all the necessary threads
 */
sdk_ret_t
thread_spawn (pds_state *state)
{
    sdk::lib::thread    *new_thread;

    /**< spawn periodic thread that does background tasks */
    new_thread = 
        thread_create(std::string("periodic").c_str(),
            THREAD_ID_PERIODIC,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            periodic_thread_start,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            NULL);
    SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                            "Periodic thread create failure");
    new_thread->start(new_thread);
    return SDK_RET_OK;
}

/**
 * install signal handler for given signal
 */
sdk_ret_t
sig_init (int signal, sig_handler_t sig_handler)
{
    struct sigaction    act;

    if (sig_handler == NULL) {
        return SDK_RET_ERR;
    }

    PDS_TRACE_DEBUG("Installing signal handler for signal %d", signal);
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(signal, &act, NULL);

    return SDK_RET_OK;
}

}    // namespace core
