/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    batch.cc
 *
 * @brief   This file deals with PDS batch related APIs
 */

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/framework/api_engine.hpp"

/**
 * @defgroup PDS_VPC_API - batch API handling
 * @ingroup PDS_VPC
 * @{
 */

/**
 * @brief    batch start API
 *
 * @param[in] batch_params    batch specific information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_batch_start (_In_ pds_batch_params_t *batch_params)
{
    if (api::g_api_engine.batching_en()) {
        return api::g_api_engine.batch_begin(batch_params);
    }
    return SDK_RET_INVALID_OP;
}

/**
 * @brief    commit the config corresponding to epoch provided in
 *           pds_batch_start() and activate the datapath to use this epoch
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_batch_commit (void)
{
    if (api::g_api_engine.batching_en()) {
        return api::g_api_engine.batch_commit();
    }
    return SDK_RET_INVALID_OP;
}

/**
 * @brief    abort the current batch processing and ignore the epoch
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t pds_batch_abort (void)
{
    if (api::g_api_engine.batching_en()) {
        return api::g_api_engine.batch_abort();
    }
    return SDK_RET_INVALID_OP;
}

/** @} */    // end of PDS_VPC_API
