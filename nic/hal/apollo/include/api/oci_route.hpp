/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_route.hpp
 *
 * @brief   This module defines route interface
 */

#if !defined (__OCI_ROUTE_HPP__)
#define __OCI_ROUTE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"

/**
 * @defgroup OCI_ROUTE - route specific API definitions
 *
 * @{
 */

// TODO: should be same as OCI_MAX_SUBNET
#define OCI_MAX_ROUTE_TABLE        (5 * OCI_MAX_VCN)
#define OCI_MAX_ROUTE_PER_TABLE    1024

/**
 * @brief nexthop types
 */
typedef enum oci_nh_type_e {
    OCI_NH_TYPE_BLACKHOLE     =  0,    /**< blackhole/drop route */
    OCI_NH_TYPE_GATEWAY       =  1,    /**< route to gateway */
    OCI_NH_TYPE_REMOTE_TEP    =  2,    /**< remote server's physical IP */
} oci_nh_type_t;

/**
 * @brief route key
 */
typedef struct oci_route_key_s {
    ip_prefix_t    prefix;      /**< IP prefix destination */
} __PACK__ oci_route_key_t;

/**
 * @brief route
 */
typedef struct oci_route_s {
    oci_route_key_t    key;        /**< route key */
    ip_addr_t          nh_ip;      /**< next hop IP address */
    oci_nh_type_t      nh_type;    /**< nexthop type */
    oci_vcn_id_t       vcn_id;     /**< result vcn id */
} __PACK__ oci_route_t;

/**
 * @brief route rules
 */
typedef struct oci_route_rules_s {
    uint32_t       num_rules;   /**< number of routes in the list */
    oci_route_t    rules[0];    /**< list or route rules */
} __PACK__ oci_route_rules_t;

/**
 * @brief route table key
 */
typedef struct oci_route_table_key_s {
    oci_route_table_id_t    id;    /**< route table id */
} __PACK__ oci_route_table_key_t;

/**
 * @brief route table
 */
typedef struct oci_route_table_s {
    oci_route_table_key_t    key;           /**< route group key */
    oci_route_rules_t        route_list;    /**< list of routes in the group */
} __PACK__ oci_route_table_t;

/**
 * @brief create route table
 *
 * @param[in] route_table route table information
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_route_table_create(_In_ oci_route_table_t *route_table);

/**
 * @brief delete route table
 *
 * @param[in] route_table_key    route table key
 * @return #SDK_RET_OK on success, failure status code on error
 */
oci_status_t oci_route_table_delete(_In_ oci_route_table_key_t *route_table_key);

/**
 * @}
 */

#endif    /** __OCI_ROUTE_HPP__ */
