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
    uint16_t id;          /**< Internal ID */

} PACKED oci_vcn_t;

/** * @} */ // end of OCI_INT_VCN

}  /** end namespace api */

#endif /** __VCN_HPP_ */
