/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.hpp
 *
 * @brief   This file deals with OCI SUBNET functionality required internally by
 *          other modules
 */

#if !defined (__SUBNET_HPP_)
#define __SUBNET_HPP_

#include "nic/sdk/include/sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace api {

/**
 * @defgroup OCI_INT_SUBNET - Internal SUBNET
 * @{
 */

/**
 * @brief Internal SUBNET structure
 */
typedef struct oci_int_subnet_s {
    oci_subnet_key_t    key;        /**< SUBNET Key */
    ht_ctxt_t           ht_ctxt;    /**< Hash table context */

    /**< P4 datapath specific state */
    uint32_t     hw_id;             /**< Internal ID */
    mem_addr_t   lpm_base_addr;     /**< LPM base address for current epoch */
    mem_addr_t   policy_base_addr;  /**< security policy rules base address for
                                         current epoch */
} __PACK__ oci_int_subnet_t;

/** * @} */ // end of OCI_INT_SUBNET

}  /** end namespace api */

#endif /** __SUBNET_HPP_ */
