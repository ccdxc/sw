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

#define PDS_ROUTE_TABLE_ID_INVALID     k_pds_obj_key_invalid

// TODO: should be same as PDS_MAX_SUBNET
#define PDS_MAX_ROUTE_TABLE            1024   ///< maximum route tables
#define PDS_MAX_ROUTE_PER_TABLE        1023   ///< maximum routes per table

/// \brief route
typedef struct pds_route_s {
    pds_obj_key_t               key;      ///< route identifier
    ip_prefix_t                 prefix;   ///< prefix
    /// NOTE:
    /// 1. priority value must be non-zero
    /// 2. lower the numerical value, higher the priority
    /// 3. a route table MUST have either all the routes with priority set or no
    ///    priority set on all the routes
    /// 4. if no priority value is set (i.e. 0) then route priority is computed
    ///    as 128 - <prefix length>, so longer prefixes will have higher
    ///    precedence over shorter ones giving regular LPM semantics
    uint32_t                    prio;     ///< priority of the route
    pds_nh_type_t               nh_type;  ///< nexthop type
    union {
        /// PDS_NH_TYPE_OVERLAY specific data
        pds_obj_key_t           tep;      ///< nexthop TEP
        /// PDS_NH_TYPE_PDS_NH_TYPE_OVERLAY_ECMP specific data
        pds_obj_key_t nh_group; ///< ECMP group
        /// PDS_NH_TYPE_PEER_VPC specific data
        pds_obj_key_t           vpc;      ///< peer vpc id
        /// one of the possible nexthop types
        pds_obj_key_t           nh;       ///< nexthop key
        /// PDS_NH_TYPE_VNIC specific data
        pds_obj_key_t           vnic;     ///< vnic nexthop
    };
    /// NAT action, if any
    /// if NAT action is set, source and/or destination NAT will be performed on
    /// the packet and if destination NAT is performed, another route lookup
    /// will be done in the same route table with the post-NAT destination IP
    /// address being rewritten
    pds_nat_action_t            nat;      ///< NAT action
    /// enable or disable metering for the route
    /// if metering is true, when traffic from a vnic hits the route and
    /// metering is enabled on that vnic, such traffic is accounted (bytes &
    /// packets) against the vnic
    bool                        meter;
} __PACK__ pds_route_t;

typedef struct route_info_s {
    uint8_t       af;         ///< address family - v4 or v6
    bool          enable_pbr; ///< enable/disable priority based routing
    uint32_t      num_routes; ///< number of routes in the list
    pds_route_t   routes[0];  ///< list or route rules
} route_info_t;
#define ROUTE_INFO_SIZE(count)        \
            (sizeof(route_info_t) + (count) * sizeof(pds_route_t))

/// \brief route table configuration
typedef struct pds_route_table_spec_s    pds_route_table_spec_t;
struct pds_route_table_spec_s {
    pds_obj_key_t key;           ///< key
    route_info_t *route_info;    ///< list of routes

    /// constructor
    pds_route_table_spec_s() { route_info = NULL; }

    /// destructor
    ~pds_route_table_spec_s() {
        if (route_info && priv_mem_) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
        }
    }

    /// copy constructor
    pds_route_table_spec_s(const pds_route_table_spec_t& route_table) {
        route_info = NULL;
        deepcopy_(route_table);
    }

    /// assignment operator
    pds_route_table_spec_t& operator= (const pds_route_table_spec_t& route_table) {
        deepcopy_(route_table);
        return *this;
    }

    /// move assignment operator
    pds_route_table_spec_t& operator= (pds_route_table_spec_t&& route_table) noexcept {
        move_(std::move(route_table));
        return *this;
    }

    /// move constructor
    pds_route_table_spec_s(pds_route_table_spec_t&& route_table) noexcept {
        route_info = NULL;
        move_(std::move(route_table));
    }

private:
    bool priv_mem_ = false;
    void deepcopy_(const pds_route_table_spec_t& route_table);
    void move_(pds_route_table_spec_t&& route_table);
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
    pds_route_table_spec_t spec;        ///< specification
    pds_route_table_status_t status;    ///< status
    pds_route_table_stats_t stats;      ///< statistics
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
sdk_ret_t pds_route_table_read(pds_obj_key_t *key,
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
sdk_ret_t pds_route_table_delete(pds_obj_key_t *key,
                                 pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief route configuration
typedef struct pds_route_spec_s {
    pds_obj_key_t key;            ///< route identifier
    pds_obj_key_t route_table;    ///< route table this route is part of
    pds_route_t route;            ///< route configuration
} pds_route_spec_t;

/// \brief route operational status
typedef struct pds_route_status_s {
} pds_route_status_t;

/// \brirf route statistics, if any
typedef struct pds_route_stats_s {
} pds_route_stats_t;

/// \brief route information
typedef struct pds_route_info_s {
    pds_route_spec_t spec;        ///< specification
    pds_route_status_t status;    ///< status
    pds_route_stats_t stats;      ///< statistics
} pds_route_info_t;

/// \brief create route
/// \param[in] spec route configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_create(pds_route_spec_t *spec,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief read route
/// \param[in] key route key
/// \param[out] info route information
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_read(pds_obj_key_t *key, pds_route_info_t *info);

/// \brief update route
/// \param[in] spec route configuration
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_update(pds_route_spec_t *spec,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete route
/// \param[in] key key of the route
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_route_delete(pds_obj_key_t *key,
                           pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);
/// @}

#endif    // __INCLUDE_API_PDS_ROUTE_HPP__
