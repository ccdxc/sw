//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains core helper functions
///
//----------------------------------------------------------------------------

#include <string>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/framework/api_thread.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "nic/apollo/pciemgr/pciemgr.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/debug.hpp"
//#include "nic/apollo/fte/fte.hpp"

using boost::property_tree::ptree;

#define SESSION_AGE_SCAN_INTVL          1     // in seconds
#define SYSTEM_SCAN_INTVL               10    // in seconds

// TODO: create a "system" class and move all this into that class
namespace core {

thread *g_thread_store[THREAD_ID_MAX];

thread *
thread_get(uint32_t thread_id) {
    if (thread_id >= THREAD_ID_MAX) {
        return NULL;
    }
    return g_thread_store[thread_id];
}

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

    // update thread library so it knows which threads to pin to
    // which cores
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

    // make sure cfg file exists
    if (access(cfg_file.c_str(), R_OK) < 0) {
        PDS_TRACE_ERR("config file %s doesn't exist or not accessible\n",
                      cfg_file.c_str());
        return SDK_RET_ERR;
    }

    // parse the config now
    std::ifstream json_cfg(cfg_file.c_str());
    read_json(json_cfg, pt);
    parse_cores_config(pt, state);

    // save pipeline
    state->set_pipeline(pipeline);
    return SDK_RET_OK;
}

sdk_ret_t
parse_global_config (string pipeline, string cfg_file, pds_state *state)
{
    ptree     pt;

    cfg_file = state->cfg_path() + pipeline + "/" + cfg_file;

    // make sure global config file exists
    if (access(cfg_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                cfg_file.c_str());
        return SDK_RET_ERR;
    }

    // parse the config now
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

sdk_ret_t
schedule_timers (pds_state *state, sdk::lib::twheel_cb_t sysmon_cb)
{
    //void    *aging_timer, *sysmon_timer;
    void *sysmon_timer;

    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    if (state->platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        // start periodic timer for scanning system interrupts, temparature,
        // power etc.
        sysmon_timer = sdk::lib::timer_schedule(
                           TIMER_ID_SYSTEM_SCAN,
                           SYSTEM_SCAN_INTVL * TIME_MSECS_PER_SEC,
                           nullptr, sysmon_cb, true);
        if (sysmon_timer == NULL) {
            PDS_TRACE_ERR("Failed to start system monitoring timer\n");
            return SDK_RET_ERR;
        }
        //PDS_TRACE_DEBUG("Started periodic system scan timer with %us intvl",
                          //SYSTEM_SCAN_INTVL);
    }
    return SDK_RET_OK;
}

// starting point for the periodic thread loop
static void *
periodic_thread_start (void *ctxt)
{
    // initialize timer wheel
    sdk::lib::periodic_thread_init(ctxt);
    // run main loop
    sdk::lib::periodic_thread_run(ctxt);

    return NULL;
}

// wrapper API to create all threads
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

// spawn all the necessary threads
sdk_ret_t
spawn_periodic_thread (pds_state *state)
{
    sdk::lib::thread    *new_thread;

    // spawn periodic thread that does background tasks
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

sdk_ret_t
spawn_pciemgr_thread (pds_state *state)
{
    sdk::lib::thread    *new_thread;

    if (state->platform_type() != platform_type_t::PLATFORM_TYPE_RTL) {
        // spawn pciemgr thread
        new_thread =
            thread_create("pciemgr", THREAD_ID_PCIEMGR,
                sdk::lib::THREAD_ROLE_CONTROL,
                0x0,    // use all control cores
                pdspciemgr::pciemgrapi::pciemgr_thread_start,
                sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                NULL);
        SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                                "pciemgr thread create failure");
        new_thread->start(new_thread);
    }
    return SDK_RET_OK;
}

sdk_ret_t
spawn_nicmgr_thread (pds_state *state)
{
    sdk::event_thread::event_thread *new_thread;

    if (state->platform_type() != platform_type_t::PLATFORM_TYPE_RTL) {
        // spawn nicmgr thread
        new_thread =
            sdk::event_thread::event_thread::factory(
                "nicmgr", THREAD_ID_NICMGR,
                sdk::lib::THREAD_ROLE_CONTROL,
                0x0,    // use all control cores
                nicmgr::nicmgrapi::nicmgr_thread_init,
                nicmgr::nicmgrapi::nicmgr_thread_exit,
                nicmgr::nicmgrapi::nicmgr_event_handler,
                sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                true);
        SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                                "nicmgr thread create failure");
        new_thread->set_data(state);
        g_thread_store[THREAD_ID_NICMGR] = new_thread;
        new_thread->start(new_thread);
    }
    return SDK_RET_OK;
}

bool
is_nicmgr_ready (void)
{
    return g_thread_store[THREAD_ID_NICMGR]->ready();
}

sdk_ret_t
spawn_api_thread (pds_state *state)
{
    sdk::event_thread::event_thread *new_thread;

    new_thread =
        sdk::event_thread::event_thread::factory(
            "cfg", THREAD_ID_API,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            api::api_thread_init_fn,
            api::api_thread_exit_fn,
            api::api_thread_event_cb,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            NULL);
     SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                             "cfg thread create failure");
     g_thread_store[THREAD_ID_API] = new_thread;
     new_thread->start(new_thread);
     return SDK_RET_OK;
}

#if 0
sdk_ret_t
spawn_fte_thread (pds_state *state)
{
    uint32_t            i, tid;
    char                thread_name[16];
    uint64_t            data_cores_mask = state->data_cores_mask();
    uint64_t            cores_mask = 0x0;
    sdk::lib::thread    *new_thread;

    if (state->platform_type() != platform_type_t::PLATFORM_TYPE_HW) {
        return SDK_RET_OK;
    }

    // spawn FTE threads
    for (i = 0; i < state->num_data_cores(); i++) {
        // pin each data thread to a specific core
        cores_mask = 1 << (ffsl(data_cores_mask) - 1);
        tid = THREAD_ID_FTE_START + i;
        snprintf(thread_name, sizeof(thread_name), "fte-%u",
                 ffsl(data_cores_mask) - 1);
        PDS_TRACE_DEBUG("Spawning FTE thread %s", thread_name);
        new_thread =
            thread_create(static_cast<const char *>(thread_name), tid,
                sdk::lib::THREAD_ROLE_DATA, cores_mask,
                fte::fte_thread_start,
                sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_DATA),
                sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_DATA),
                NULL);
        SDK_ASSERT_TRACE_RETURN((new_thread != NULL), SDK_RET_ERR,
                                "%s thread create failure",
                                thread_name);
        new_thread->start(new_thread);
        data_cores_mask = data_cores_mask & (data_cores_mask-1);
    }
    return SDK_RET_OK;
}
#endif

// stop the threads
void
threads_stop (void)
{
    int thread_id;

    for (thread_id = 0; thread_id < THREAD_ID_MAX; thread_id++) {
        if (g_thread_store[thread_id] != NULL) {
            // stop the thread
            g_thread_store[thread_id]->stop();
            PDS_TRACE_DEBUG("Stoppig thread %s", g_thread_store[thread_id]->name());
        }
    }
    for (thread_id = 0; thread_id < THREAD_ID_MAX; thread_id++) {
        if (g_thread_store[thread_id] != NULL) {
            g_thread_store[thread_id]->wait();
            // free the allocated thread
            sdk::lib::thread::destroy(g_thread_store[thread_id]);
            g_thread_store[thread_id] = NULL;
        }
    }
}

// install signal handler for given signal
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
