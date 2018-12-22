/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_mapping.hpp
 *
 * @brief   This module defines mapping interface
 */

#if !defined (__OCI_MAPPING_HPP__)
#define __OCI_MAPPING_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_MAPPING - mapping specific API definitions
 *
 * @{
 */

typedef struct oci_mapping_key_s {
    oci_mapping_id_t    id;    /**< (locally) unique IP mapping identifier */
} __PACK__ oci_mapping_key_t;

/**
 * @brief IP mapping information
 */
typedef struct oci_mapping_s {
    oci_mapping_key_t    key;                  /**< mapping key */
    ip_addr_t            overlay_ip;           /**< Overlay/Private IP address */
    ip_addr_t            public_ip;            /**< Public IP address */
    uint32_t             public_ip_valid:1;    /**< TRUE if public IP is valid */
    bool                 local;                /**< true if this is for local vnic */
    union {
        oci_vnic_id_t    vnic_id;              /**< vnic id for local local mapping */
        ipv4_addr_t      tep;                  /**< Tunnel dst behind which this IP is */
    };
} oci_mapping_t;

/**
 * @brief Create IP mapping
 *
 * @param[in] mapping        IP mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_mapping_create(_In_ oci_mapping_t *mapping);

/**
 * @brief Delete IP mapping
 *
 * @param[in] mapping    IP mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_mappnig_delete(_In_ oci_mapping_t *mapping);

/**
 * @}
 */

#endif    /** __OCI_MAPPING_HPP__ */
