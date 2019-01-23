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
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_tep.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"
#include "nic/apollo/include/api/oci_vnic.hpp"

/**
 * @defgroup OCI_MAPPING - mapping specific API definitions
 *
 * @{
 */

typedef struct oci_mapping_key_s {
    oci_vcn_key_t    vcn;        /**< vcn this IP belongs to */
    ip_addr_t        ip_addr;    /**< IP address of the mapping */
} __PACK__ oci_mapping_key_t;

/**
 * @brief IP mapping information
 */
typedef struct oci_mapping_s {
    oci_mapping_key_t     key;                /**< mapping key */
    oci_subnet_key_t      subnet;             /**< subnet this IP is part of */
    oci_slot_id_t         slot;               /**< Virtual slot
                                                   (Encap: MPLS Tag) */
    oci_tep_key_t         tep;                /**< TEP address for this mapping
                                                   1. switchport IP for local vnic
                                                   2. remote TEP for remote vnic */
    mac_addr_t            overlay_mac;        /**< MAC for this IP */

    /**< info specific to local IP mappings */
    struct {
        oci_vnic_key_t    vnic;               /**< vnic for local IP */
        bool              public_ip_valid;    /**< TRUE if public IP is valid */
        ip_addr_t         public_ip;          /**< Public IP address */
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
