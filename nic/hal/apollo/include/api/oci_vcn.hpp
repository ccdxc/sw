/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vcn.hpp
 *
 * @brief   This module defines OCI VCN interface
 */

#if !defined (__OCI_VCN_HPP__)
#define __OCI_VCN_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_VCN - Virtual Cloud Network specific API definitions
 *
 * @{
 */

#define OCI_MAX_VCN        1024

/**
 * @brief VCN Key
 */
typedef struct oci_vcn_key_s {
    oci_vcn_id_t id;    /**< VNC ID */
} __PACK__ oci_vcn_key_t;

typedef enum oci_vcn_type_e {
    OCI_VCN_TYPE_SUBSTRATE = 0,    /**< substrate VCN */
    OCI_VCN_TYPE_TENANT    = 1,    /**< tenant/customer VCN */
} oci_vcn_type_t;

/**
 * @brief VCN
 */
typedef struct oci_vcn_s {
    oci_vcn_type_t    type;    /**< VCN type */
    oci_vcn_key_t     key;     /**< VCN Key */
    ip_prefix_t       pfx;     /**< VCN CIDR block */
} __PACK__ oci_vcn_t;

/**
 * @brief Create VCN
 *
 * @param[in] vcn VCN information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vcn_create(_In_ oci_vcn_t *vcn);

/**
 * @brief Delete VCN
 *
 * @param[in] vcn_key VCN key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vcn_delete(_In_ oci_vcn_key_t *vcn_key);

/**
 * @}
 */

#endif    /** __OCI_VCN_HPP__ */
