/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    core.cc
 *
 * @brief   This file contains core helper functions
 */

#if !defined (__OCI_CORE_HPP__)
#define __OCI_CORE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"

namespace core {

enum {
    THREAD_ID_NONE       = 0,
    THREAD_ID_CONTROL    = 1,
    THREAD_ID_PERIODIC   = 2,
    THREAD_ID_DATA_START = 16,
    THREAD_ID_DATA_END   = 47,
    THREAD_ID_MAX        = 38
};

sdk_ret_t thread_spawn(oci_state *state);
sdk_ret_t parse_global_config(string cfg_file, oci_state *state);
sdk_ret_t parse_pipeline_config(const char *cfgfile, oci_state *state);

}    // namespace core

#endif    /** __OCI_CORE_HPP__ */
