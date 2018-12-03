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

/**
 * @defgroup OCI_VNIC - VNIC specific API definitions
 *
 * @{
 */

#define OCI_MAX_VNIC               1024
#define OCI_MAX_IP_PER_VNIC        32

/**
 * @brief VNIC IP Information
 */
typedef struct oci_vnic_ip_s {
    ip_addr_t    overlay_ip;          /**< Overlay/Private IP address */
    ip_addr_t    public_ip;           /**< Public IP address */
    uint32_t     public_ip_valid:1;    /**< TRUE if public IP is valid */
} oci_vnic_ip_t;

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
    // TODO: kalyan, why are these not oci_vcn_key_t and
    // oci_subnet_key_t ???
    oci_vcn_id_t       vcn_id;                   /**< VCN ID */
    oci_subnet_id_t    subnet_id;                /**< Subnet ID */
    oci_vnic_key_t     key;                      /**< VNIC Key */
    uint16_t           vlan_id;                  /**< VLAN ID */
    oci_slot_id_t      slot;                     /**< Virtual slot
                                                      (Encap: MPLS Tag) */
    mac_addr_t         mac_addr;                 /**< MAC address */
    oci_rsc_pool_id_t  rsc_pool_id;              /**< Resource pool associated
                                                      with this VNIC */
    uint16_t           num_ips;                  /**< no. of IPs for this VNIC */
    oci_vnic_ip_t      ip_info[OCI_MAX_IP_PER_VNIC]; /**< IP information */
    uint32_t           src_dst_check : 1;        /**< TRUE if source/destination
                                                      check is enabled */
    uint32_t           local : 1;                /**< TRUE if vnic is local */
    ipv4_addr_t        tep;                      /**< Tunnel dst behind which the
                                                      VNIC is present */
} __PACK__ oci_vnic_t;

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_vnic_create(_In_ oci_vnic_t *vnic);

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_vnic_delete(_In_ oci_vnic_key_t *vnic_key);

/**
 * @}
 */

#endif    /** __OCI_VNIC_HPP__ */
