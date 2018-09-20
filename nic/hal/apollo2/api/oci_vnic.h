/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vnic.h
 *
 * @brief   This module defines OCI VNIC interface
 */

#if !defined (__OCI_VNIC_H_)
#define __OCI_VNIC_H_

#include <oci_types.h>

/**
 * @defgroup OCI_VNIC - VNIC specific API definitions
 *
 * @{
 */

/**
 * @brief VNIC IP Information
 */
typedef struct _oci_vnic_ip_t
{
    /**
     * @brief Private IP Address
     */
    oci_ip_addr_t private_ip;

    /**
     * @brief Public IP Address
     */
    oci_ip_addr_t public_ip;

    /**
     * @brief Substrate IP Address
     */
    oci_ip_addr_t substrate_ip;

    /**
     * @brief TRUE if public IP is valid
     */
    uint32_t public_ip_valid:1;

    /**
     * @brief TRUE if substrate IP is valid
     */
    uint32_t substrate_ip_valid:1;

} vnic_ip_info_t;

/**
 * @brief VNIC Key
 */
typedef struct _oci_vnic_key_t
{
    /**
     * @brief VCN ID
     */
    oci_vcn_id_t vcn_id;

    /**
     * @brief Subnet ID
     */
    oci_subnet_id_t subnet_id;

    /**
     * @brief VNIC ID
     */
    oci_vnic_id_t vnic_id;

} PACKED oci_vnic_key_t;

/**
 * @brief VNIC
 */
typedef struct _oci_vnic_t
{
    /**
     * @brief VNIC Key
     */
    oci_vnic_key_t key;

    /**
     * @brief VLAN ID
     */
    oci_vlan_id_t vlan_id;

    /**
     * @brief Virtual slot (Encap: MPLS Tag)
     */
    oci_slot_id_t slot;

    /**
     * @brief MAC address
     */
    oci_mac_t mac_addr;

    /**
     * @brief IP information
     */
    oci_vnic_ip_t ip_info[MAX_IP_PER_VNIC];

    /**
     * @brief Source/Destination check is enabled
     */
    uint32_t src_dst_check : 1;

} PACKED oci_vnic_t;

/**
 * @brief Create VNIC 
 *
 * @param[in] vnic VNIC information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_create (
        _In_ oci_vnic_t *vnic);


/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_vnic_delete (
        _In_ oci_vnic_key_t *vnic_key);

/**
 * @}
 */
#endif /** __OCI_VNIC_H_ */
