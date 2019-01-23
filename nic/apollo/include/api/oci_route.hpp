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
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/oci.hpp"

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
 * @brief route
 */
typedef struct oci_route_s {
    ip_prefix_t        prefix;     /**< route prefix */
    ip_addr_t          nh_ip;      /**< next hop IP address */
    oci_nh_type_t      nh_type;    /**< nexthop type */
    oci_vcn_id_t       vcn_id;     /**< result vcn id */
} __PACK__ oci_route_t;

/**
 * @brief route table key
 */
typedef struct oci_route_table_key_s {
    oci_route_table_id_t    id;    /**< route table id */
} __PACK__ oci_route_table_key_t;

/**
 * @brief route table
 */
typedef struct oci_route_table_s    oci_route_table_t;
struct oci_route_table_s {
    oci_route_table_key_t    key;          /**< route group key */
    uint8_t                  af;           /**< address family - IP_AF_IPV4 or
                                                IP_AF_IPV6 */
    uint32_t                 num_routes;   /**< number of routes in the list */
    oci_route_t              *routes;      /**< list or route rules */

    oci_route_table_s() { routes = NULL; }
    ~oci_route_table_s() {
        if (routes) {
            SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE, routes);
        }
    }
    oci_route_table_t& operator= (const oci_route_table_t& route_table) {
        /**< self-assignment guard */
        if (this == &route_table) {
            return *this;
        }
        key = route_table.key;
        af = route_table.af;
        num_routes = route_table.num_routes;
        if (routes) {
            SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE, routes);
        }
        routes =
            (oci_route_t *)SDK_MALLOC(OCI_MEM_ALLOC_ROUTE_TABLE,
                                      num_routes * sizeof(oci_route_t));
        memcpy(routes, route_table.routes, num_routes * sizeof(oci_route_t));
        return *this;
    }
} __PACK__;

/**
 * @brief create route table
 *
 * @param[in] route_table route table information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_route_table_create(_In_ oci_route_table_t *route_table);

/**
 * @brief delete route table
 *
 * @param[in] route_table_key    route table key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t oci_route_table_delete(_In_ oci_route_table_key_t *route_table_key);

/**
 * @}
 */

#endif    /** __OCI_ROUTE_HPP__ */
