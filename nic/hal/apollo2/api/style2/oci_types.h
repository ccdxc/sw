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

typedef int32_t  oci_status_t;
typedef uint32_t oci_attr_id_t;
typedef uint8_t  oci_mac_t[6];
typedef uint32_t oci_ip4_t;
typedef uint8_t  oci_ip6_t[16];

#define _In_
#define _Out_
#define _Inout_
#define _In_reads_z_(_LEN_)
#define _In_reads_opt_z_(_LEN_)


/*
 * New common definitions
 */

typedef uint64_t oci_uint64_t;
typedef int64_t oci_int64_t;
typedef uint32_t oci_uint32_t;
typedef int32_t oci_int32_t;
typedef uint16_t oci_uint16_t;
typedef int16_t oci_int16_t;
typedef uint8_t oci_uint8_t;
typedef int8_t oci_int8_t;
typedef size_t oci_size_t;
typedef uint64_t oci_object_id_t;
typedef void *oci_pointer_t;

typedef struct _oci_timespec_t
{
    uint64_t tv_sec;
    uint32_t tv_nsec;
} oci_timespec_t;

/**
 * @def OCI_NULL_OBJECT_ID
 * OCI NULL object ID
 */
#define OCI_NULL_OBJECT_ID 0L

/**
 * @brief Defines a list of OCI object ids used as OCI attribute value.
 *
 * In set attribute function call, the count member defines the number of
 * objects.
 *
 * In get attribute function call, the function call returns a list of objects
 * to the caller in the list member. The caller is responsible for allocating the
 * buffer for the list member and set the count member to the size of allocated object
 * list. If the size is large enough to accommodate the list of object id, the
 * callee will then fill the list member and set the count member to the actual
 * number of objects. If the list size is not large enough, the callee will set the
 * count member to the actual number of object id and return
 * #OCI_STATUS_BUFFER_OVERFLOW. Once the caller gets such return code, it should
 * use the returned count member to re-allocate list and retry.
 */
typedef struct _oci_object_list_t
{
    uint32_t count;
    oci_object_id_t *list;
} oci_object_list_t;

/**
 * @brief OCI common API type
 */
typedef enum _oci_common_api_t
{
    OCI_COMMON_API_CREATE      = 0,
    OCI_COMMON_API_REMOVE      = 1,
    OCI_COMMON_API_SET         = 2,
    OCI_COMMON_API_GET         = 3,
    OCI_COMMON_API_BULK_CREATE = 4,
    OCI_COMMON_API_BULK_REMOVE = 5,
    OCI_COMMON_API_BULK_SET    = 6,
    OCI_COMMON_API_BULK_GET    = 7,
    OCI_COMMON_API_MAX         = 8,
} oci_common_api_t;

/**
 * @brief OCI object type
 */
typedef enum _oci_object_type_t
{
    OCI_OBJECT_TYPE_NULL                     =  0, /**< invalid object type */
    OCI_OBJECT_TYPE_VCN                      =  1,
    OCI_OBJECT_TYPE_SUBNET                   =  2,
    OCI_OBJECT_TYPE_VNIC                     =  3,
    OCI_OBJECT_TYPE_TUNNEL                   =  4,
    OCI_OBJECT_TYPE_ROUTE                    =  5,
    OCI_OBJECT_TYPE_MAX                      =  6,
} oci_object_type_t;

typedef struct _oci_u8_list_t
{
    uint32_t count;
    uint8_t *list;
} oci_u8_list_t;

/**
 * @brief Defines a s8 list or string
 *
 * String should be null terminated and count should include '\0'.
 */
typedef struct _oci_s8_list_t
{
    uint32_t count;
    int8_t *list;
} oci_s8_list_t;

typedef struct _oci_u16_list_t
{
    uint32_t count;
    uint16_t *list;
} oci_u16_list_t;

typedef struct _oci_s16_list_t
{
    uint32_t count;
    int16_t *list;
} oci_s16_list_t;

typedef struct _oci_u32_list_t
{
    uint32_t count;
    uint32_t *list;
} oci_u32_list_t;

typedef struct _oci_s32_list_t
{
    uint32_t count;
    int32_t *list;
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
    oci_ip_address_t *list;
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
    oci_vnicipinfo_t *list;

} oci_vnicipinfo_list_t;

/**
 * @brief Data Type
 *
 * To use enum values as attribute value is oci_int32_t s32
 *
 * @extraparam const oci_attr_metadata_t *meta
 */
typedef union _oci_attribute_value_t
{
    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_BOOL */
    bool booldata;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_CHARDATA */
    char chardata[32];

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT8 */
    oci_uint8_t u8;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT8 */
    oci_int8_t s8;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT16 */
    oci_uint16_t u16;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT16 */
    oci_int16_t s16;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT32 */
    oci_uint32_t u32;

    /**
     * @suffix enum
     * @passparam meta->enummetadata
     * @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT32
     */
    oci_int32_t s32;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT64 */
    oci_uint64_t u64;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT64 */
    oci_int64_t s64;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_POINTER */
    oci_pointer_t ptr;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_MAC */
    oci_mac_t mac;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_IPV4 */
    oci_ip4_t ip4;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_IPV6 */
    oci_ip6_t ip6;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_IP_ADDRESS */
    oci_ip_address_t ipaddr;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_IP_PREFIX */
    oci_ip_prefix_t ipprefix;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_OBJECT_ID */
    oci_object_id_t oid;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_OBJECT_LIST */
    oci_object_list_t objlist;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT8_LIST */
    oci_u8_list_t u8list;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT8_LIST */
    oci_s8_list_t s8list;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT16_LIST */
    oci_u16_list_t u16list;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT16_LIST */
    oci_s16_list_t s16list;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT32_LIST */
    oci_u32_list_t u32list;

    /**
     * @suffix enum_list
     * @passparam meta->enummetadata
     * @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT32_LIST
     */
    oci_s32_list_t s32list;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_UINT32_RANGE */
    oci_u32_range_t u32range;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_INT32_RANGE */
    oci_s32_range_t s32range;

    /** @validonly meta->attrvaluetype == SAI_ATTR_VALUE_TYPE_VNIC_IPINFO_LIST */
    oci_vnic_ipinfo_list_t vnicipinfolist;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_IP_ADDRESS_LIST */
    oci_ip_address_list_t ipaddrlist;

    /** @validonly meta->attrvaluetype == OCI_ATTR_VALUE_TYPE_TIMESPEC */
    oci_timespec_t timespec;
} oci_attribute_value_t;

/**
 * @extraparam const oci_attr_metadata_t *meta
 */
typedef struct _oci_attribute_t
{
    /** @passparam meta */
    oci_attr_id_t id;

    /** @passparam meta */
    oci_attribute_value_t value;
} oci_attribute_t;

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
