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
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

/// \defgroup PDS_ROUTE Route API
/// @{

// TODO: should be same as PDS_MAX_SUBNET
#define PDS_MAX_ROUTE_TABLE            1024
#define PDS_MAX_ROUTE_PER_TABLE        1023

/// \brief nexthop type
typedef enum pds_nh_type_e {
    PDS_NH_TYPE_NONE      = 0,
    PDS_NH_TYPE_BLACKHOLE = 1,    ///< blackhole/drop nexthop
    PDS_NH_TYPE_TEP       = 2,    ///< any of the possible types of TEP
    PDS_NH_TYPE_PEER_VPC  = 3,    ///< VPC id of the peer VPC
} pds_nh_type_t;

/// \brief route
typedef struct pds_route_s {
    ip_prefix_t          prefix;     ///< prefix
    pds_nh_type_t        nh_type;    ///< nexthop type
    union {
        ip_addr_t        nh_ip;      ///< nexthop IP address
        pds_vpc_key_t    vpc;        ///< peer vpc id, in case of vpc peering
    };
} __PACK__ pds_route_t;

/// \brief route table configuration
typedef struct pds_route_table_spec_s    pds_route_table_spec_t;
struct pds_route_table_spec_s {
    pds_route_table_key_t    key;          ///< key
    uint8_t                  af;           ///< address family - v4 or v6
    uint32_t                 num_routes;   ///< number of routes in the list
    pds_route_t              *routes;      ///< list or route rules

    // constructor
    pds_route_table_spec_s() { routes = NULL; }

    // destructor
    ~pds_route_table_spec_s() {
        if (routes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, routes);
        }
    }

    // assignment operator
    pds_route_table_spec_t& operator= (const pds_route_table_spec_t& route_table) {
        // self-assignment guard
        if (this == &route_table) {
            return *this;
        }
        key = route_table.key;
        af = route_table.af;
        num_routes = route_table.num_routes;
        if (routes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, routes);
        }
        routes = (pds_route_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                           num_routes * sizeof(pds_route_t));
        memcpy(routes, route_table.routes, num_routes * sizeof(pds_route_t));
        return *this;
    }
} __PACK__;

/// \brief route table status
typedef struct pds_route_table_status_s {
    // TODO : Only base address of the tree stored in HBM is read
} pds_route_table_status_t;

/// \brief route table information
typedef struct pds_route_table_info_s {
    pds_route_table_spec_t spec;            ///< Specification
    pds_route_table_status_t status;        ///< Status
} __PACK__ pds_route_table_info_t;

/// \brief create route table
/// \param[in] spec route table configuration
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_create(pds_route_table_spec_t *spec);

/// \brief delete route table
/// \param[in] key key
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_delete(pds_route_table_key_t *key);

/// \@}

#endif    // __INCLUDE_API_PDS_ROUTE_HPP__
