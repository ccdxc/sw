/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api.hpp
 *
 * @brief   This file provides generic API processing related macros,
 *          enums, APIs etc.
 */

#if !defined (__API_HPP__)
#define __API_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/sdk/include/sdk/ht.hpp"

using sdk::lib::ht;
using sdk::lib::ht_ctxt_t;
using sdk::lib::indexer;
using sdk::lib::slab;
using sdk::lib::hash_algo;

namespace api {

/**
 * @brief    API operation
 */
typedef enum api_op_e {
    API_OP_NONE,
    API_OP_CREATE,
    API_OP_DELETE,
    API_OP_UPDATE,
    API_OP_GET,
} api_op_t;

typedef struct api_ctxt_s {
    api_op_t    op;
} api_ctxt_t;

/**
 * @brief    base class for all objects
 */
class oci_base {
public:
    oci_base(){};
    ~oci_base(){};
protected:
    /**
     * @brief    process a create/delete/update/get operation on a vcn
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_api(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

    /**
     * @brief    commit() is invokved during commit phase of the API processing
     *           and is not expected to fail as all required resources are
     *           already allocated by now. Based on the API operation, this API
     *           is expected to process either create/retrieve/update/delete. If
     *           any temporary state was stashed in the api_ctxt while
     *           processing this API, it should be freed here
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     *
     * NOTE:     commit() is not expected to fail
     */
    virtual sdk_ret_t commit(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

    /**
     * @brief     abort() is invoked during abort phase of the API processing
     *            and is not expected to fail. During this phase, all associated
     *            resources must be freed and global DBs need to be restored
     *            back to their original state and any transient state stashed
     *            in api_ctxt while processing this API should also be freed
     *            here
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t abort(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

};

}    // namespace api

#endif /** __API_HPP_ */
