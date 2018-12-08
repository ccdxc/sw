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
sdk_ret_t
oci_init (oci_init_params_t *params)
{
    sdk::lib::logger::init(params->debug_trace_cb, params->error_trace_cb);
    // TODO:
    // 1. asic_init()
    // 2. pipeline_init()
    // 3. linkmgr_init()
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
