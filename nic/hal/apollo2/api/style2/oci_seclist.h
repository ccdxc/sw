/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file   oci_seclist.h
 *
 * @brief   This module defines OCI security list interface
 */

#if !defined (__OCI_SECLIST_H_)
#define __OCI_SECLIST_H_

#include <oci_types.h>

/**
 * @defgroup OCI_SECLIST - Security List specific API definitions
 *
 * @{
 */

/**
 * @brief ACL IP Type
 */
typedef enum _oci_acl_ip_type_t
{
    /** Don't care */
    OCI_ACL_IP_TYPE_ANY,

    /** IPv4 and IPv6 packets */
    OCI_ACL_IP_TYPE_IP,

    /** Non-IP packet */
    OCI_ACL_IP_TYPE_NON_IP,

    /** Any IPv4 packet */
    OCI_ACL_IP_TYPE_IPV4ANY,

    /** Anything but IPv4 packets */
    OCI_ACL_IP_TYPE_NON_IPV4,

    /** IPv6 packet */
    OCI_ACL_IP_TYPE_IPV6ANY,

    /** Anything but IPv6 packets */
    OCI_ACL_IP_TYPE_NON_IPV6,

    /** ARP/RARP */
    OCI_ACL_IP_TYPE_ARP,

    /** ARP Request */
    OCI_ACL_IP_TYPE_ARP_REQUEST,

    /** ARP Reply */
    OCI_ACL_IP_TYPE_ARP_REPLY

} oci_acl_ip_type_t;

/**
 * @brief ACL Action Type
 */
typedef enum _oci_acl_action_type_t
{
    /** Set Redirect */
    OCI_ACL_ACTION_TYPE_REDIRECT,

} oci_acl_action_type_t;

    /**
     * @brief Range type defined
     *
     * @type oci_s32_list_t oci_acl_range_type_t
     * @flags CREATE_ONLY
     * @default empty
     */
    OCI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE,

/**
 * @brief Attribute Id for oci_sec_rule
 *
 * @flags Contains flags
 */
typedef enum _oci_sec_rule_t
{
    /**
     * @brief Start of Security Rule attributes
     */
    OCI_SEC_RULE_ATTR_START,

    /**
     * @brief OCI Security Rule id
     *
     * @type oci_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @objects OCI_OBJECT_TYPE_ACL_TABLE
     */
    OCI_SEC_RULE_ATTR_ID = OCI_SEC_RULE_ATTR_START,

    /**
     * @brief Priority
     *
     * Value must be in the range defined in
     * \[#OCI_SEC_RULE_MINIMUM_PRIORITY,
     * #OCI_SEC_RULE_MAXIMUM_PRIORITY\]
     * (default = #OCI_SEC_RULE_MINIMUM_PRIORITY)
     *
     * @type oci_uint32_t
     * @flags CREATE_AND_SET
     * @default 0
     */
    OCI_SEC_RULE_ATTR_PRIORITY,

    /**
     * @brief Admin state
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default true
     */
    OCI_SEC_RULE_ATTR_ADMIN_STATE,

    /*
     * Match fields [oci_sec_rule_match_data_t]
     * - Mandatory to pass at least one match field during security rule creation.
     * - Unless noted specifically, both data and mask are required.
     * - When bit field is used, only those least significant bits are valid for
     * matching.
     */

    /**
     * @brief Start of Rule Match Fields
     */
    OCI_SEC_RULE_MATCH_ATTR_START = 0x00001000,

    /**
     * @brief Src IPv6 Address
     *
     * @type oci_sec_rule_match_data_t oci_ip6_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_SRC_IPV6 = OCI_SEC_RULE_MATCH_ATTR_START,

    /**
     * @brief Dst IPv6 Address
     *
     * @type oci_sec_rule_match_data_t oci_ip6_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_ATTR_FIELD_DST_IPV6,

    /**
     * @brief Src IPv4 Address
     *
     * @type oci_sec_rule_match_data_t oci_ip4_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_SRC_IP,

    /**
     * @brief Dst IPv4 Address
     *
     * @type oci_sec_rule_match_data_t oci_ip4_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_DST_IP,

    /**
     * @brief L4 Src Port
     *
     * @type oci_sec_rule_match_data_t oci_uint16_t
     * @flags CREATE_AND_SET
     * @isvlan false
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_L4_SRC_PORT,

    /**
     * @brief L4 Dst Port
     *
     * @type oci_sec_rule_match_data_t oci_uint16_t
     * @flags CREATE_AND_SET
     * @isvlan false
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_L4_DST_PORT,

    /**
     * @brief IP Protocol
     *
     * @type oci_sec_rule_match_data_t oci_uint8_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_IP_PROTOCOL,

    /**
     * @brief ICMP Type
     *
     * @type oci_sec_rule_match_data_t oci_uint8_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_ICMP_TYPE,

    /**
     * @brief ICMP Code
     *
     * @type oci_sec_rule_match_data_t oci_uint8_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_ICMP_CODE,

    /**
     * @brief Range Type defined in oci_acl_range_type_t
     *
     * List of OCI ACL Range Object Id
     *
     * @type oci_sec_rule_match_data_t oci_object_list_t
     * @flags CREATE_AND_SET
     * @objects OCI_OBJECT_TYPE_ACL_RANGE
     * @default disabled
     */
    OCI_SEC_RULE_MATCH_ATTR_ACL_RANGE_TYPE,

    /**
     * @brief End of Rule Match Fields
     */
    OCI_SEC_RULE_MATCH_ATTR_END = OCI_SEC_RULE_MATCH_ATTR_IPV6_NEXT_HEADER,

    /*
     * Actions [oci_acl_action_data_t]
     *
     * - To enable an action, parameter is needed unless noted specifically.
     * - To disable an action, parameter is not needed.
     */

    /**
     * @brief Start of Rule Actions
     */
    OCI_ACL_ENTRY_ATTR_ACTION_START = 0x00002000,

    /**
     * @brief Drop Packet
     *
     * @type oci_acl_action_data_t oci_packet_action_t
     * @flags CREATE_AND_SET
     * @default disabled
     */
    OCI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION,

    /**
     * @brief End of Rule Actions
     */
    OCI_ACL_ENTRY_ATTR_ACTION_END = OCI_ACL_ENTRY_ATTR_ACTION_SET_ISOLATION_GROUP,

    /**
     * @brief End of ACL Entry attributes
     */
    OCI_ACL_ENTRY_ATTR_END,

} oci_sec_rule_attr_t;

/**
 * @brief Attribute data for Security Rule Range Type
 */
typedef enum _oci_sec_rule_range_type_t
{
    /** L4 Source Port Range */
    OCI_SEC_RULE_RANGE_TYPE_L4_SRC_PORT_RANGE,

    /** L4 Destination Port Range */
    OCI_SEC_RULE_RANGE_TYPE_L4_DST_PORT_RANGE,

} oci_sec_rule_range_type_t;

/**
 * @brief Attribute Id for Security Rule Range Object
 */
typedef enum _oci_acl_range_attr_t
{
    /**
     * @brief Start of attributes
     */
    OCI_ACL_RANGE_ATTR_START,

    /**
     * @brief Range type
     *
     * Mandatory to pass only one of the range types
     * defined in oci_acl_range_type_t enum during ACL Range Creation.
     * Range Type cannot be changed after the range is created.
     *
     * @type oci_acl_range_type_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_ACL_RANGE_ATTR_TYPE = OCI_ACL_RANGE_ATTR_START,

    /**
     * @brief Start and End of ACL Range
     *
     * Range will include the start and end values.
     * Range Limit cannot be changed after the range is created.
     *
     * @type oci_u32_range_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    OCI_ACL_RANGE_ATTR_LIMIT,

    /**
     * @brief End of attributes
     */
    OCI_ACL_RANGE_ATTR_END,

    /** Custom range base value */
    OCI_ACL_RANGE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    OCI_ACL_RANGE_ATTR_CUSTOM_RANGE_END

} oci_acl_range_attr_t;

/**
 * @brief Create Rule
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 * @param[in] route_entry Route entry
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_create (
        _In_ const oci_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _In_ const oci_attribute_t *attr_list);

/**
 * @brief Set route attribute value
 *
 * @param[in] route_entry Route entry
 * @param[in] attr Attribute
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_attribute_set (
        _In_ const oci_route_entry_t *route_entry,
        _In_ const oci_attribute_t *attr);

/**
 * @brief Get route attribute value
 *
 * @param[in] route_entry Route entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_attribute_get (
        _In_ const oci_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _Inout_ oci_attribute_t *attr_list);

/**
 * @brief Delete Route
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 * @param[in] route_entry Route entry
 *
 * @return #OCI_STATUS_SUCCESS on success, failure status code on error
 */
oci_status_t oci_route_entry_delete (
        _In_ const oci_route_entry_t *route_entry);

/**
 * @brief Bulk create route entry
 *
 * @param[in] object_count Number of objects to create
 * @param[in] route_entry List of object to create
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to create.
 * @param[in] attr_list List of attributes for every object.
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are created or
 * #OCI_STATUS_FAILURE when any of the objects fails to create. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_bulk_create (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const uint32_t *attr_count,
        _In_ const oci_attribute_t **attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk set attribute on route entry
 *
 * @param[in] object_count Number of objects to set attribute
 * @param[in] route_entry List of objects to set attribute
 * @param[in] attr_list List of attributes to set on objects, one attribute per object
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_attribute_bulk_set (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const oci_attribute_t *attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk get attribute on route entry
 *
 * @param[in] object_count Number of objects to set attribute
 * @param[in] route_entry List of objects to set attribute
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to get
 * @param[inout] attr_list List of attributes to set on objects, one attribute per object
 * @param[in] mode Bulk operation error handling mode
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_attribute_bulk_get (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ const uint32_t *attr_count,
        _Inout_ oci_attribute_t **attr_list,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @brief Bulk delete route entry
 *
 * @param[in] object_count Number of objects to remove
 * @param[in] route_entry List of objects to remove
 * @param[in] mode Bulk operation error handling mode.
 * @param[out] object_statuses List of status for every object. Caller needs to
 * allocate the buffer
 *
 * @return #OCI_STATUS_SUCCESS on success when all objects are removed or
 * #OCI_STATUS_FAILURE when any of the objects fails to remove. When there is
 * failure, Caller is expected to go through the list of returned statuses to
 * find out which fails and which succeeds.
 */
oci_status_t oci_route_entry_bulk_delete (
        _In_ uint32_t object_count,
        _In_ const oci_route_entry_t *route_entry,
        _In_ oci_bulk_op_error_mode_t mode,
        _Out_ oci_status_t *object_statuses);

/**
 * @}
 */
#endif /** __OCI_SECLIST_H_ */
