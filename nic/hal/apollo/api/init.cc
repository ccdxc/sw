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
    asic_cfg.asic_type = sdk::asic::SDK_ASIC_TYPE_CAPRI;
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
