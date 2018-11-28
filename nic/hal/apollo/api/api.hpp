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
    API_OP_RETRIEVE,
    API_OP_UDPATE,
    API_OP_DELETE,
} api_op_t;

}    // namespace api

#endif /** __API_HPP_ */
