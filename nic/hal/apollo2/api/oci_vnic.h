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
    oci_ip_addr_t private_ip;         /**< Private IP address */
    oci_ip_addr_t public_ip;          /**< Public IP address */
    uint32_t public_ip_valid:1;       /**< TRUE if public IP is valid */

} oci_vnic_ip_t;

/**
 * @brief VNIC Key
 */
typedef struct _oci_vnic_key_t
{
    oci_vcn_id_t vcn_id;          /**< VCN ID */
    oci_subnet_id_t subnet_id;    /**< Subnet ID */
    oci_vnic_id_t id;             /**< VNIC ID */

} PACKED oci_vnic_key_t;

/**
 * @brief VNIC
 */
typedef struct _oci_vnic_t
{
    oci_vnic_key_t key;                        /**< VNIC Key */
    oci_vlan_id_t vlan_id;                     /**< VLAN ID */
    oci_slot_id_t slot;                        /**< Virtual slot
                                                    (Encap: MPLS Tag) */
    oci_mac_t mac_addr;                        /**< MAC address */
    oci_resource_pool_id_t resource_pool_od;   /**< resource pool this vnic is associated with */
    oci_vnic_ip_t ip_info[MAX_IP_PER_VNIC];    /**< IP information */
    uint32_t src_dst_check : 1;                /**< TRUE if source/destination
                                                    check is enabled */
    uint32_t local : 1;                        /**< TRUE if vnic is local */
    oci_ip_addr_t tep;                         /**< Tunnel dst behind which the
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
