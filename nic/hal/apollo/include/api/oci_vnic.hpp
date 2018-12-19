/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vnic.hpp
 *
 * @brief   This module defines OCI VNIC interface
 */

#if !defined (__OCI_VNIC_HPP__)
#define __OCI_VNIC_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"

/**
 * @defgroup OCI_VNIC - VNIC specific API definitions
 *
 * @{
 */

#define OCI_MAX_VNIC               1024
/**
 * @brief VNIC Key
 */
typedef struct oci_vnic_key_s {
    oci_vnic_id_t id;             /**< VNIC ID (in the range 0 to 4095)
                                       NOTE: this id must be locally unique
                                             across all VCNs */
} __PACK__ oci_vnic_key_t;

/**
 * @brief VNIC
 */
typedef struct oci_vnic_s {
    oci_vcn_key_t       vcn_id;                   /**< VCN ID */
    oci_subnet_key_t    subnet_id;                /**< Subnet ID */
    oci_vnic_key_t      key;                      /**< VNIC Key */
    uint16_t            wire_vlan;                /**< VLAN ID */
    oci_slot_id_t       slot;                     /**< Virtual slot
                                                       (Encap: MPLS Tag) */
    mac_addr_t          mac_addr;                 /**< MAC address */
    oci_rsc_pool_id_t   rsc_pool_id;              /**< Resource pool associated
                                                       with this VNIC */
    bool                src_dst_check;            /**< TRUE if source/destination
                                                       check is enabled */
    uint32_t            local;                    /**< TRUE if vnic is local */
#if 0
    uint16_t           num_ips;                  /**< no. of IPs for this VNIC */
    oci_vnic_ip_t      ip_info[OCI_MAX_IP_PER_VNIC]; /**< IP information */
#endif
} __PACK__ oci_vnic_t;

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vnic_create(_In_ oci_vnic_t *vnic);

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vnic_delete(_In_ oci_vnic_key_t *vnic_key);

#define OCI_MAX_IP_PER_VNIC        32

/**
 * @brief VNIC IP mapping information
 */
typedef struct oci_vnic_ip_mapping_s {
    oci_vnic_key_t    key;                  /**< VNIC Key */
    ip_addr_t         overlay_ip;           /**< Overlay/Private IP address */
    ip_addr_t         public_ip;            /**< Public IP address */
    uint32_t          public_ip_valid:1;    /**< TRUE if public IP is valid */
    ipv4_addr_t       tep;                  /**< Tunnel dst behind which the
                                                 VNIC is present */
    // TODO: what about slot ??
} oci_vnic_ip_mapping_t;

/**
 * @brief Create VNIC's IP mapping
 *
 * @param[in] vnic_ip_mapping        VNIC's IP mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vnic_ip_mapping_create(_In_ oci_vnic_ip_mapping_t *vnic_ip_mapping);

/**
 * @brief Delete VNIC's IP mapping
 *
 * @param[in] oci_vnic_ip_mapping    VNIC's IP mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_vnic_ip_mappnig_delete(_In_ oci_vnic_ip_mapping_t *vnic_ip_mapping);

/**
 * @}
 */

#endif    /** __OCI_VNIC_HPP__ */
