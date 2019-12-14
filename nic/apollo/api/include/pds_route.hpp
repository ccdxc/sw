//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines route table APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_ROUTE_HPP__
#define __INCLUDE_API_PDS_ROUTE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

/// \defgroup PDS_ROUTE Route API
/// @{

// TODO: should be same as PDS_MAX_SUBNET
#define PDS_MAX_ROUTE_TABLE            1024   ///< maximum route tables
#define PDS_MAX_ROUTE_PER_TABLE        1023   ///< maximum routes per table

/// \brief route
typedef struct pds_route_s {
    ip_prefix_t                 prefix;   ///< prefix
    pds_nh_type_t               nh_type;  ///< nexthop type
    /// NOTE:
    /// 1. priority value must be non-zero
    /// 2. lower the numerical value, higher the priority
    /// 3. a route table MUST have either all the routes with priority set or no
    ///    priority set on all the routes
    /// 4. if no priority value is set (i.e. 0) then route priority is computed
    ///    as 128 - <prefix length>, so longer prefixes will have higher
    ///    precedence over shorter ones giving regular LPM semantics
    uint32_t                    prio;     ///< priority of the route
    union {
        /// PDS_NH_TYPE_OVERLAY specific data
        pds_tep_key_t           tep;      ///< nexthop TEP
        /// PDS_NH_TYPE_PDS_NH_TYPE_OVERLAY_ECMP specific data
        pds_nexthop_group_key_t nh_group; ///< ECMP group
        /// PDS_NH_TYPE_PEER_VPC specific data
        pds_vpc_key_t           vpc;      ///< peer vpc id
        /// one of the possible nexthop types
        pds_nexthop_key_t       nh;       ///< nexthop key
        /// PDS_NH_TYPE_VNIC specific data
        pds_vnic_key_t          vnic;     ///< vnic nexthop
    };
    /// NAT action, if any
    /// if NATAction is set, source and/or destination NAT will be performed on
    /// the packet and if destination NAT is performed, another route lookup
    /// will be done in the same route table with the post-NAT destination IP
    /// address being rewritten
    pds_nat_action_t        nat;          ///< NAT action
} __PACK__ pds_route_t;

/// \brief route table configuration
typedef struct pds_route_table_spec_s    pds_route_table_spec_t;
/// \brief route table configuration
struct pds_route_table_spec_s {
    pds_route_table_key_t key;        ///< key
    uint8_t               af;         ///< address family - v4 or v6
    bool                  enable_pbr; ///< enable/disable priority based routing
    uint32_t              num_routes; ///< number of routes in the list
    pds_route_t           *routes;    ///< list or route rules

    // constructor
    pds_route_table_spec_s() { routes = NULL; }

    // destructor
    ~pds_route_table_spec_s() {
        if (routes) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, routes);
        }
    }

    /// assignment operator
    pds_route_table_spec_t& operator= (const pds_route_table_spec_t& route_table) {
        // self-assignment guard
        if (this == &route_table) {
            return *this;
        }
        key = route_table.key;
        af = route_table.af;
        enable_pbr = route_table.enable_pbr;
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
    mem_addr_t route_table_base_addr;       ///< route table base address
} pds_route_table_status_t;

/// \brief route table statistics
typedef struct pds_route_table_stats_s {

} pds_route_table_stats_t;

/// \brief route table information
typedef struct pds_route_table_info_s {
    pds_route_table_spec_t spec;            ///< Specification
    pds_route_table_status_t status;        ///< Status
    pds_route_table_stats_t stats;          ///< Statistics
} __PACK__ pds_route_table_info_t;

/// \brief create route table
/// \param[in] spec route table configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_create(pds_route_table_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read route table
/// \param[in] key route table key
/// \param[out] info route table information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_read(pds_route_table_key_t *key,
                               pds_route_table_info_t *info);

/// \brief update route table
/// \param[in] spec route table configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_update(pds_route_table_spec_t *spec,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete route table
/// \param[in] key key
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_table_delete(pds_route_table_key_t *key,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_ROUTE_HPP__
