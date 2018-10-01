/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.hpp
 *
 * @brief   This file deals with OCI VCN functionality required internally by
 *          other modules
 */

#if !defined (__VCN_HPP_)
#define __VCN_HPP_

#include "nic/sdk/include/sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace api {

/**
 * @defgroup OCI_INT_VCN - Internal VCN
 * @{
 */

/**
 * @brief Internal VCN structure
 */
typedef struct oci_int_vcn_s
{
    oci_vcn_key_t key;    /**< VCN Key */
    uint32_t id;          /**< Internal ID */
    ht_ctxt_t ht_ctxt;    /**< Hash table context */

} PACKED oci_int_vcn_t;

/** * @} */ // end of OCI_INT_VCN

}  /** end namespace api */

#endif /** __VCN_HPP_ */
