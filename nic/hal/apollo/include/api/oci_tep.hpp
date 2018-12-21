/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_tep.hpp
 *
 * @brief   This module defines Tunnel EndPoint (TEP) interface
 */

#if !defined (__OCI_TEP_HPP__)
#define __OCI_TEP_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_TEP - Tunnel specific API definitions
 *
 * @{
 */

#define OCI_MAX_TEP                1024

/**
 * @brief Defines encap type
 */
typedef enum oci_encap_type_e {
    OCI_ENCAP_TYPE_IPINIP_GRE = 0,    /**< GRE tunnel */
} oci_encap_type_t;

/**
 * @brief Tunnel Key
 */
typedef struct oci_tep_key_s {
    ipv4_addr_t    ip_addr;    /**< Tunnel IP address */
} __PACK__ oci_tep_key_t;

/**
 * @brief Tunnel
 */
typedef struct oci_tep_s {
    oci_tep_key_t       key;     /**< Tunnel key */
    oci_encap_type_t    type;    /**< Tunnel type */
} __PACK__ oci_tep_t;

/**
 * @brief Create tep
 *
 * @param[in] tep Tunnel information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_tep_create(_In_ oci_tep_t *tep);


/**
 * @brief Delete tep
 *
 * @param[in] tep_key Tunnel Key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_tep_delete(_In_ oci_tep_key_t *tep_key);

/**
 * @}
 */

#endif    /** __OCI_TEP_HPP__ */
