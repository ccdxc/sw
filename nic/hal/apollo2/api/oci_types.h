/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_types.h
 *
 * @brief   This module defines OCI portable types
 */

#if !defined (__OCI_TYPES_H_)
#define __OCI_TYPES_H_

/**
 * @defgroup OCI_TYPES - Types definitions
 *
 * @{
 */


#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define _In_
#define _Out_
#define _Inout_
#define PACKED __attribute__((__packed__))

/*
 * New common definitions
 */

typedef uint64_t    oci_uint64_t;
typedef int64_t     oci_int64_t;
typedef uint32_t    oci_uint32_t;
typedef int32_t     oci_int32_t;
typedef uint16_t    oci_uint16_t;
typedef int16_t     oci_int16_t;
typedef uint8_t     oci_uint8_t;
typedef int8_t      oci_int8_t;
typedef size_t      oci_size_t;
typedef void        *oci_pointer_t;

typedef int32_t     oci_status_t;
typedef uint32_t    oci_vnc_id_t;
typedef uint32_t    oci_subnet_id_t;
typedef uint32_t    oci_vnic_id_t;
typedef uint32_t    oci_rule_id_t;
typedef uint16_t    oci_vlan_id_t;
typedef uint32_t    oci_slot_id_t;    // 20 bit id
typedef uint8_t     oci_mac_t[6];
typedef uint32_t    oci_ip4_t;
typedef uint8_t     oci_ip6_t[16];

typedef struct _oci_timespec_t
{
    uint64_t tv_sec;
    uint32_t tv_nsec;
} oci_timespec_t;

typedef struct _oci_u8_list_t
{
    uint32_t count;
    uint8_t  list[0];
} oci_u8_list_t;

typedef struct _oci_u16_list_t
{
    uint32_t count;
    uint16_t list[0];
} oci_u16_list_t;

typedef struct _oci_s16_list_t
{
    uint32_t count;
    int16_t list[0];
} oci_s16_list_t;

typedef struct _oci_u32_list_t
{
    uint32_t count;
    uint32_t list[0];
} oci_u32_list_t;

typedef struct _oci_s32_list_t
{
    uint32_t count;
    int32_t list[0];
} oci_s32_list_t;

typedef struct _oci_u32_range_t
{
    uint32_t min;
    uint32_t max;
} oci_u32_range_t;

typedef struct _oci_s32_range_t
{
    int32_t min;
    int32_t max;
} oci_s32_range_t;

typedef enum _oci_ip_addr_family_t
{
    OCI_IP_ADDR_FAMILY_IPV4,

    OCI_IP_ADDR_FAMILY_IPV6

} oci_ip_addr_family_t;

/**
 * @extraparam oci_ip_addr_family_t addr_family
 */
typedef union _oci_ip_addr_t
{
    /** @validonly addr_family == OCI_IP_ADDR_FAMILY_IPV4 */
    oci_ip4_t ip4;

    /** @validonly addr_family == OCI_IP_ADDR_FAMILY_IPV6 */
    oci_ip6_t ip6;
} oci_ip_addr_t;

typedef struct _oci_ip_address_t
{
    oci_ip_addr_family_t addr_family;

    /** @passparam addr_family */
    oci_ip_addr_t addr;
} oci_ip_address_t;

typedef struct _oci_ip_address_list_t
{
    uint32_t count;
    oci_ip_address_t list[0];
} oci_ip_address_list_t;

typedef struct _oci_ip_prefix_t
{
    oci_ip_addr_family_t addr_family;

    /** @passparam addr_family */
    oci_ip_addr_t addr;

    /** @passparam addr_family */
    oci_ip_addr_t mask;
} oci_ip_prefix_t;

/**
 * @brief Structure for VNIC IP Info
 */
typedef struct _oci_vnicipinfo_t
{
    /** Private IP Address */
    oci_ip_addr_t private_ip;

    /** Public IP Address */
    oci_ip_addr_t public_ip;

    /** Substrate IP Address */
    oci_ip_addr_t substrate_ip;

    /** Is public IP valid? */
    bool public_ip_valid;

    /** Is substrate IP valid? */
    bool substrate_ip_valid;

} oci_vnicipinfo_t;


/**
 * @brief List of VNIC IP Info.
 */
typedef struct _oci_vnicipinfo_list_t
{
    /** Number of entries */
    oci_uint32_t count;

    /** VNIC IPInfo List */
    oci_vnicipinfo_t list[0];

} oci_vnicipinfo_list_t;


typedef enum _oci_bulk_op_error_mode_t
{
    /**
     * @brief Bulk operation error handling mode where operation stops on the first failed creation
     *
     * Rest of objects will use OCI_STATUS_NON_EXECUTED return status value.
     */
    OCI_BULK_OP_ERROR_MODE_STOP_ON_ERROR,

    /**
     * @brief Bulk operation error handling mode where operation ignores the failures and continues to create other objects
     */
    OCI_BULK_OP_ERROR_MODE_IGNORE_ERROR,
} oci_bulk_op_error_mode_t;

typedef enum _oci_stats_mode_t
{
    /**
     * @brief Read statistics
     */
    OCI_STATS_MODE_READ,

    /**
     * @brief Read and clear after reading
     */
    OCI_STATS_MODE_READ_AND_CLEAR,
} oci_stats_mode_t;

/**
 * @}
 */
#endif /** __OCI_TYPES_H_ */
