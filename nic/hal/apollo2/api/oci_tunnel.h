/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_tunnel.h
 *
 * @brief   This module defines OCI Tunnel interface
 */

#if !defined (__OCI_TUNNEL_H_)
#define __OCI_TUNNEL_H_

#include <oci_types.h>

/**
 * @defgroup OCI_TUNNEL - Tunnel specific API definitions
 *
 * @{
 */

/**
 * @brief Defines tunnel type
 */
typedef enum _oci_tunnel_type_t
{
    OCI_TUNNEL_TYPE_IPINIP_GRE,

} oci_tunnel_type_t;

/**
 * @brief Tunnel Key
 */
typedef struct _oci_tunnel_key_t
{
    /**
     * @brief Tunnel destination
     */
    oci_ip4_t dst;

} PACKED oci_tunnel_key_t;

/**
 * @brief Tunnel
 */
typedef struct _oci_tunnel_t
{
    /**
     * @brief Tunnel Key
     */
    oci_tunnel_key_t key;

    /**
     * @brief Tunnel type
     */
    oci_tunnel_type_t type;

} PACKED oci_tunnel_t;

/**
 * @brief Create tunnel
 *
 * @param[in] tunnel Tunnel information
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_tunnel_create (
        _In_ oci_tunnel_t *tunnel);


/**
 * @brief Delete tunnel
 *
 * @param[in] tunnel_key Tunnel Key
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_tunnel_delete (
        _In_ oci_tunnel_key_t *tunnel_key);

/**
 * @}
 */
#endif /** __OCI_TUNNEL_H_ */
