/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_switchport.h
 *
 * @brief   This module defines OCI SWITCHPORT interface
 */

#if !defined (__OCI_SWITCHPORT_H_)
#define __OCI_SWITCHPORT_H_

#include <oci_types.h>

/**
 * @defgroup OCI_SWITCHPORT - Switchport Params specific API definitions
 *
 * @{
 */

/**
 * @brief SWITCHPORT
 */
typedef struct _oci_switchport_t
{
    oci_ip_addr_t    switch_ip_addr;    /**< SWITCHPORT IP Address */
    oci_mac_addr_t   switch_mac_addr; /**< SWITCHPORT Mac Addr */ 
    oci_ip_addr_t    gateway_ip_addr;    /**< Gateway IP Address */
} PACKED oci_switchport_t;

/**
 * @brief Create SWITCHPORT
 *
 * @param[in] switchport SWITCHPORT information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_switchport_create (
        _In_ oci_switchport_t *switchport);


/**
 * @brief Delete SWITCHPORT
 *
 * @param[in] switchport SWITCHPORT 
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_switchport_delete (
        _In_ oci_switchport_t *switchport);

/**
 * @}
 */
#endif /** __OCI_SWITCHPORT_H_ */
