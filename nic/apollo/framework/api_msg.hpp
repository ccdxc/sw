//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API message definition and helper functions
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_MSG_HPP__
#define __FRAMEWORK_API_MSG_HPP__

#include <vector>
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

using std::vector;

namespace api {

/// \brief    IPC msgs handled by cfg thread
typedef enum api_msg_id_e {
    API_MSG_ID_NONE,
    API_MSG_ID_BATCH,
} api_msg_id_t;

///< API batch message containing bunch of API calls in single batch
typedef struct batch_info_s {
    ///< epoch is the config version to advance to
    pds_epoch_t           epoch;
    ///< process this batch synchronously or asynchronously
    bool                  async;
    ///< batch cookie is something that client uses to correlate
    ///< API batch request and response in case batch is requested
    ///< to be processed asynchronously
    void                  *cookie;
    ///< the callback to call once this batch finishes processing
    pds_async_rsp_cb_t    response_cb;
    ///< list of api calls to process in this batch
    vector<api_ctxt_t *> apis;
} batch_info_t;

/// \brief    IPC message sent to config thread for API processing
typedef struct api_msg_s {
    api_msg_id_t msg_id;
    // valid if msg_id is API_MSG_BATCH
    batch_info_t batch;
} api_msg_t;

slab *api_msg_slab(void);

/// \brief start the batch for commit
/// \param[in] batch_params Batch specific information
/// \return opaque ctxt to (internally) identify the batch
pds_batch_ctxt_t api_batch_start(pds_batch_params_t *batch_params);

/// \brief commit the configuration in the batch
/// commit the configuration corresponding to epoch provided
/// in pds_batch_start() and activate the datapath to use this epoch
/// \param[in] opaque batch context identifying the API batching
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t api_batch_commit(pds_batch_ctxt_t bctxt);

/// \brief destroy/abort all the batch context
/// \param[in] opaque batch context identifying the API batching
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t api_batch_destroy(pds_batch_ctxt_t bctxt);

/// \brief    wrapper function to process all API calls
/// \param[in] msg_id      unique message identifier
/// \param[in] bctxt       API batch context
/// \param[in] api_ctxt    api specific context to be added to batch or
///                        processed individually if batch context in invalid
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t process_api(pds_batch_ctxt_t bctxt, api_ctxt_t *api_ctxt);

}    // namespace api

using api::api_msg_id_t;
using api::api_msg_t;

#endif    // __FRAMEWORK_API_MSG_HPP__
