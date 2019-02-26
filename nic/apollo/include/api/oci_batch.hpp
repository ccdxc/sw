//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines batching API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_BATCH_HPP__
#define __INCLUDE_API_OCI_BATCH_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_BATCH Batch API
/// @{

// TODO: there is a case where epoch can rollover and new epoch is actually
//       less than old epoch, how does datapath handle this ? In s/w if we
//       detect this, we can set old epoch to OCI_INVALID_EPOCH in all table
//       entries we are touching
/// \brief Batch parameters
typedef struct oci_batch_params_s {
    oci_epoch_t    epoch;    ///< Epoch value for this batch
                             ///< OCI_EPOCH_INVALID is reserved
} __PACK__ oci_batch_params_t;

/// \brief Start the batch for commit
///
/// \param[in] batch_params Batch specific information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t oci_batch_start(oci_batch_params_t *batch_params);

/// \brief Commit the configuration in the batch
///
/// Commit the configuration corresponding to epoch provided
/// in oci_batch_start() and activate the datapath to use this epoch
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t oci_batch_commit(void);

/// \brief Aborts currents batch processing and ignore the epoch
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t oci_batch_abort(void);

/// \@}

#endif    // __INCLUDE_API_OCI_BATCH_HPP__
