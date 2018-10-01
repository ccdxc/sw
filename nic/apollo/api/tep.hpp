/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep.hpp
 *
 * @brief   This file deals with OCI TEP functionality required internally by
 *          other modules
 */

#if !defined (__TEP_HPP_)
#define __TEP_HPP_

#include "nic/sdk/include/sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace api {

/**
 * @defgroup OCI_INT_TEP - Internal TEP
 * @{
 */

/**
 * @brief Internal TEP structure
 */
typedef struct oci_int_tep_s
{
    oci_tep_key_t key;    /**< TEP Key */
    uint32_t id;          /**< Internal ID */
    ht_ctxt_t ht_ctxt;    /**< Hash table context */

} PACKED oci_int_tep_t;

/** * @} */ // end of OCI_INT_TEP

}  /** end namespace api */

#endif /** __TEP_HPP_ */
