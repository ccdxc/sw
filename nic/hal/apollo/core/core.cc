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
#include "nic/hal/apollo/core/trace.hpp"
#include "nic/hal/apollo/core/core.hpp"

using boost::property_tree::ptree;
using std::string;

namespace core {

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
        fprintf(stderr, "HAL config file %s doesn't exist or not accessible\n",
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

}    // namespace core
