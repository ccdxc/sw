//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines route API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_ROUTE_HPP__
#define __INCLUDE_API_PDS_ROUTE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/include/api/pds.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"

/// \defgroup PDS_ROUTE Route API
/// @{

// TODO: should be same as PDS_MAX_SUBNET
#define PDS_MAX_ROUTE_TABLE            (5 * PDS_MAX_VCN)
#define PDS_MAX_ROUTE_PER_TABLE        1023

/// \brief Nexthop type
typedef enum pds_nh_type_e {
    PDS_NH_TYPE_BLACKHOLE     =  0,    ///< blackhole/drop route
    PDS_NH_TYPE_GATEWAY       =  1,    ///< route to gateway
    PDS_NH_TYPE_REMOTE_TEP    =  2,    ///< remote server's physical IP
} pds_nh_type_t;

/// \brief Route
typedef struct pds_route_s {
    ip_prefix_t        prefix;     ///< Prefix
    ip_addr_t          nh_ip;      ///< Nexthop IP address
    pds_nh_type_t      nh_type;    ///< Nexthop type
    pds_vcn_id_t       vcn_id;     ///< Result VCN ID
} __PACK__ pds_route_t;

/// \brief Route table key
/// \remark
///  - route table id is not scoped under vcn, it is unique across vcns
typedef struct pds_route_table_key_s {
    pds_route_table_id_t    id;    ///< Route table ID
} __PACK__ pds_route_table_key_t;

/// \brief Route table
typedef struct pds_route_table_s    pds_route_table_t;
struct pds_route_table_s {
    pds_route_table_key_t    key;          ///< Key
    uint8_t                  af;           ///< Address family - v4 or v6
    uint32_t                 num_routes;   ///< Number of routes in the list
    pds_route_t              *routes;      ///< List or route rules

    /// Constructor
    pds_route_table_s() { routes = NULL; }

    /// Destructor
    ~pds_route_table_s() {
        if (routes) {
            SDK_FREE(PDS_MEM_ALLOC_ROUTE_TABLE, routes);
        }
    }

    //// Assignment
    pds_route_table_t& operator= (const pds_route_table_t& route_table) {
        // self-assignment guard
        if (this == &route_table) {
            return *this;
        }
        key = route_table.key;
        af = route_table.af;
        num_routes = route_table.num_routes;
        if (routes) {
            SDK_FREE(PDS_MEM_ALLOC_ROUTE_TABLE, routes);
        }
        routes = (pds_route_t *)SDK_MALLOC(PDS_MEM_ALLOC_ROUTE_TABLE,
                                           num_routes * sizeof(pds_route_t));
        memcpy(routes, route_table.routes, num_routes * sizeof(pds_route_t));
        return *this;
    }
} __PACK__;

/// \brief Create route table
///
/// \param[in] route_table route table information
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_create(pds_route_table_t *route_table);

/// \brief Delete route table
///
/// \param[in] key Key
///
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_delete(pds_route_table_key_t *route_table_key);

/// \@}

#endif    // __INCLUDE_API_PDS_ROUTE_HPP__
