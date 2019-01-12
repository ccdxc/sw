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
#include "nic/hal/apollo/core/trace.hpp"
#include "nic/hal/apollo/core/core.hpp"

using boost::property_tree::ptree;
using std::string;

namespace core {

thread *g_thread_store[THREAD_ID_MAX];

static sdk_ret_t
parse_cores_config (ptree &pt, oci_state *state)
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
parse_pipeline_config (const char *cfgfile, oci_state *state)
{
    ptree     pt;
    string    cfg_file;

    cfg_file = state->cfg_path() + "apollo/" + string(cfgfile);

    /**< make sure cfg file exists */
    if (access(cfg_file.c_str(), R_OK) < 0) {
        OCI_TRACE_ERR("config file %s doesn't exist or not accessible\n",
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
parse_global_config (string cfg_file, oci_state *state)
{
    ptree     pt;

    cfg_file = state->cfg_path() + "apollo/" + cfg_file;

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
thread_spawn (oci_state *state)
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

}    // namespace core
