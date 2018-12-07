/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_batch.hpp
 *
 * @brief   This module defines batching APIs
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

#if !defined (__OCI_BATCH_HPP__)
#define __OCI_BATCH_HPP__

/**
 * @defgroup OCI_BATCH - batch APIs
 *
 * @{
 */

// TODO: there is a case where epoch can rollover and new epoch is actually
//       less than old epoch, how does datapath handle this ? In s/w if we
//       detect this, we can set old epoch to OCI_INVALID_EPOCH in all table
//       entries we are touching
typedef struct oci_batch_params_s {
    oci_epoch_t    epoch;    /**< epoch value for this batch
                                  NOTE: OCI_EPOCH_INVALID is reserved */
} __PACK__ oci_batch_params_t;

/**
 * @brief    batch start API
 *
 * @param[in] batch_params    batch specific information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_batch_start(_In_ oci_batch_params_t *batch_params);

/**
 * @brief    commit the config corresponding to epoch provided in
 *           oci_batch_start() and activate the datapath to use this epoch
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_batch_commit(void);

/**
 * @brief    abort the current batch processing and ignore the epoch
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_batch_abort(void);

/**
 * @}
 */
#endif    /** __OCI_BATCH_HPP__ */
