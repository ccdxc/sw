/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_switchport.hpp
 *
 * @brief   This module defines switchport interface, which is global
 *          configuration, this is the first piece of configuration provided
 *          by the agent to this library (and can't be deleted)
 */

#if !defined (__OCI_SWITCHPORT_HPP__)
#define __OCI_SWITCHPORT_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_SWITCHPORT - Switchport Params specific API definitions
 *
 * @{
 */

/**
 * @brief SWITCHPORT
 */
typedef struct oci_switchport_s {
    ipv4_addr_t    switch_ip_addr;     /**< SWITCHPORT IP Address */
    mac_addr_t     switch_mac_addr;    /**< SWITCHPORT Mac Addr */ 
    ipv4_addr_t    gateway_ip_addr;    /**< Gateway IP Address */
} __PACK__ oci_switchport_t;

/**
 * @brief Create switchport configuration
 *
 * @param[in] switchport global switchport configuration
 *
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_switchport_create(_In_ oci_switchport_t *switchport);

/**
 * @}
 */

#endif    /** __OCI_SWITCHPORT_HPP__ */
