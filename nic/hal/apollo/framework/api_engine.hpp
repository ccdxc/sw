/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_engine.hpp
 *
 * @brief   API processing framework/engine functionality
 */

#if !defined (__API_ENGINE_HPP__)
#define __API_ENGINE_HPP__

#include <vector>

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/include/api/oci_batch.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

using std::vector;

namespace api {

/**
 * @brief   processing stage of the APIs in a given batch
 */
typedef enum api_batch_stage_e {
    API_BATCH_STAGE_NONE,
    API_BATCH_STAGE_INIT,
    API_BATCH_STAGE_RSC_ALLOC,    /**< s/w & h/w resource allocation stage */
    API_BATCH_STAGE_COMMIT,       /**< commit stage */
    API_BATCH_STAGE_ABORT,        /**< abort stage */
} api_batch_stage_t;

/**
 * @brief    per batch context which is a list of all API contexts
 */
typedef struct api_batch_ctxt_s {
    oci_epoch_t           epoch;    /**< epoch in progress, passed in
                                         oci_batch_begin() */
    api_batch_stage_t     stage;    /**< phase of the batch processing */
    vector<api_ctxt_t>    apis;     /**< list of API objects being
                                         processed */
} api_batch_ctxt_t;

/**
 * @brief    encapsulation for all API processing framework
 */
class api_engine {
public:
    /**
     * @brief    constructor
     */
    api_engine() {};

    /**
     * @brief    destructor
     */
    ~api_engine() {};

    /**
     * @brief    handle batch begin by setting up per API batch context
     */
    sdk_ret_t batch_begin(oci_batch_params_t *batch_params);

    /**
     * @brief    commit all the APIs in this batch, release any temporary
     *           state or resources like memory, per API context info etc.
     */
    sdk_ret_t batch_commit(void);

    /**
     * @brief    abort all the APIs in this batch, release any temporary
     *           state or resources like memory, per API context info etc.
     */
    sdk_ret_t batch_abort(void);

    /**
     * @brief    wrapper function for processing all API calls
     */
    sdk_ret_t process_api(api_ctxt_t *api_ctxt);

private:
    api_batch_ctxt_t    batch_ctxt_;
};

/**< API engine (singleton) instance */
extern api_engine    g_api_engine;

}    // namespace api

#endif    /** __API_ENGINE_HPP__ */
