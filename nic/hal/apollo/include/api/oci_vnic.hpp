/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vnic.hpp
 *
 * @brief   This module defines vnic interface
 */

#if !defined (__OCI_VNIC_HPP__)
#define __OCI_VNIC_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"

/**
 * @defgroup OCI_VNIC - vnic specific API definitions
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
    oci_vcn_key_t       vcn;              /**< vcn of this vnic */
    oci_subnet_key_t    subnet;           /**< subnet of this vnic */
    oci_vnic_key_t      key;              /**< vnic's Key */
    uint16_t            wire_vlan;        /**< vlan tag assigned to this vnic */
    oci_slot_id_t       slot;             /**< virtual slot
                                               (Encap: MPLS Tag) */
    mac_addr_t          mac_addr;         /**< vnic's overlay mac mac address */
    oci_rsc_pool_id_t   rsc_pool_id;      /**< resource pool associated
                                               with this vnic */
    bool                src_dst_check;    /**< TRUE if source/destination
                                               check is enabled */
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

/**
 * @}
 */

#endif    /** __OCI_VNIC_HPP__ */
