//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines route API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_ROUTE_HPP__
#define __INCLUDE_API_OCI_ROUTE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/oci.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"

/// \defgroup OCI_ROUTE Route API
/// @{

// TODO: should be same as OCI_MAX_SUBNET
#define OCI_MAX_ROUTE_TABLE            (5 * OCI_MAX_VCN)
#define OCI_MAX_ROUTE_PER_TABLE        1023

/// \brief Nexthop type
typedef enum oci_nh_type_e {
    OCI_NH_TYPE_BLACKHOLE     =  0,    ///< blackhole/drop route
    OCI_NH_TYPE_GATEWAY       =  1,    ///< route to gateway
    OCI_NH_TYPE_REMOTE_TEP    =  2,    ///< remote server's physical IP
} oci_nh_type_t;

/// \brief Route
typedef struct oci_route_s {
    ip_prefix_t        prefix;     ///< Prefix
    ip_addr_t          nh_ip;      ///< Nexthop IP address
    oci_nh_type_t      nh_type;    ///< Nexthop type
    oci_vcn_id_t       vcn_id;     ///< Result VCN ID
} __PACK__ oci_route_t;

/// \brief Route table key
/// \remark
///  - route table id is not scoped under vcn, it is unique across vcns
typedef struct oci_route_table_key_s {
    oci_route_table_id_t    id;    ///< Route table ID
} __PACK__ oci_route_table_key_t;

/// \brief Route table
typedef struct oci_route_table_s    oci_route_table_t;
struct oci_route_table_s {
    oci_route_table_key_t    key;          ///< Key
    uint8_t                  af;           ///< Address family - v4 or v6
    uint32_t                 num_routes;   ///< Number of routes in the list
    oci_route_t              *routes;      ///< List or route rules

    /// Constructor
    oci_route_table_s() { routes = NULL; }

    /// Destructor
    ~oci_route_table_s() {
        if (routes) {
            SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE, routes);
        }
    }

    //// Assignment
    oci_route_table_t& operator= (const oci_route_table_t& route_table) {
        // self-assignment guard
        if (this == &route_table) {
            return *this;
        }
        key = route_table.key;
        af = route_table.af;
        num_routes = route_table.num_routes;
        if (routes) {
            SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE, routes);
        }
        routes = (oci_route_t *)SDK_MALLOC(OCI_MEM_ALLOC_ROUTE_TABLE,
                                           num_routes * sizeof(oci_route_t));
        memcpy(routes, route_table.routes, num_routes * sizeof(oci_route_t));
        return *this;
    }
} __PACK__;

/// \brief Create route table
///
/// \param[in] route_table route table information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_route_table_create(oci_route_table_t *route_table);

/// \brief Delete route table
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t oci_route_table_delete(oci_route_table_key_t *route_table_key);

/// \@}

#endif    // __INCLUDE_API_OCI_ROUTE_HPP__
