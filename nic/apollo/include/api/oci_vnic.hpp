//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines VNIC API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_VNIC_HPP__
#define __INCLUDE_API_OCI_VNIC_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"

/// \defgroup OCI_VNIC VNIC API
/// @{

#define OCI_MAX_VNIC 1024

/// \brief VNIC key
typedef struct oci_vnic_key_s {
    oci_vnic_id_t id;    ///< Unique VNIC ID (in the range 0 to 1024)
} __PACK__ oci_vnic_key_t;

/// \brief VNIC specification
typedef struct oci_vnic_spec_s {
    oci_vcn_key_t vcn;                ///< VCN of this vnic
    oci_subnet_key_t subnet;          ///< Subnet of this vnic
    oci_vnic_key_t key;               ///< VNIC's Key
    uint16_t wire_vlan;               ///< VLAN tag assigned to this vnic
    oci_slot_id_t slot;               ///< Virtual slot (Encap: MPLS Tag)
    mac_addr_t mac_addr;              ///< VNIC's overlay mac mac address
    oci_rsc_pool_id_t rsc_pool_id;    ///< Resource pool
    bool src_dst_check;               ///< TRUE if src/dst check is enabled
} __PACK__ oci_vnic_spec_t;

/// \brief VNIC status
typedef struct oci_vnic_status_s {
    // TODO
} oci_vnic_status_t;

/// \brief VNIC statistics
typedef struct oci_vnic_stats_s {
    uint64_t rx_pkts;     ///< Received packet count
    uint64_t rx_bytes;    ///< Received bytes
    uint64_t tx_pkts;     ///< Transmit packet count
    uint64_t tx_bytes;    ///< Transmit bytes
} oci_vnic_stats_t;

/// \brief VNIC information
typedef struct oci_vnic_info_s {
    oci_vnic_spec_t spec;        ///< VNIC specification
    oci_vnic_status_t status;    ///< VNIC status
    oci_vnic_stats_t stats;      ///< VNIC stats
} oci_vnic_info_t;

/// \brief Create VNIC
///
/// \param[in] spec Specification
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_vnic_create(oci_vnic_spec_t *spec);

/// \brief Read VNIC information
///
/// \param[in] key Key
/// \param[out] info Information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_vnic_read(oci_vnic_key_t *key, oci_vnic_info_t *info);

/// \brief Update VNIC specification
///
/// \param[in] spec Specififcation
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_vnic_update(oci_vnic_spec_t *spec);

/// \brief Delete VNIC
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_vnic_delete(oci_vnic_key_t *key);

/// @}

#endif    ///  __INCLUDE_API_OCI_VNIC_HPP__
