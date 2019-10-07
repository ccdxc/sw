//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines batching API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_BATCH_HPP__
#define __INCLUDE_API_PDS_BATCH_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_BATCH Batch API
/// @{

/// \brief    api batch parameters
typedef struct pds_batch_params_s {
    pds_epoch_t    epoch;    ///< Epoch value for this batch
                             ///< PDS_EPOCH_INVALID is reserved
    bool           async;    ///< process this batch asynchronously
                             ///< and return the batch cookie along with
                             ///< the response
    uint64_t       cookie;   ///< cookie to correlate batch request with
                             ///< response in case of async processing
} __PACK__ pds_batch_params_t;

/// \brief start the batch for commit
/// \param[in] batch_params Batch specific information
/// \return opaque ctxt to (internally) identify the batch
pds_batch_ctxt_t pds_batch_start(pds_batch_params_t *batch_params);

/// \brief commit the configuration in the batch
/// commit the configuration corresponding to epoch provided
/// in pds_batch_start() and activate the datapath to use this epoch
/// \param[in] opaque batch context identifying the API batching
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt);

/// \brief destroy/abort all the batch context
/// \param[in] opaque batch context identifying the API batching
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt);

/// @}

#endif    // __INCLUDE_API_PDS_BATCH_HPP__
