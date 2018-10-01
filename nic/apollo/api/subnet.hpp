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

namespace api {

/**
 * @defgroup OCI_INT_SUBNET - Internal SUBNET
 * @{
 */

/**
 * @brief Internal SUBNET structure
 */
typedef struct oci_int_subnet_s
{
    oci_sbunet_key_t key;    /**< SUBNET Key */
    uint16_t id;             /**< Internal ID */

} PACKED oci_subnet_t;

/** * @} */ // end of OCI_INT_SUBNET

}  /** end namespace api */

#endif /** __SUBNET_HPP_ */
