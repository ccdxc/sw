//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines Flow APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_FLOW_HPP__
#define __INCLUDE_API_PDS_FLOW_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/vpp/flow/pdsa_hdlr.h"

/// \brief flow key
typedef struct pds_flow_key_s {
    uint32_t lookup_id;
    ip_addr_t src_ip;
    ip_addr_t dst_ip;
    uint16_t proto;
    uint16_t sport;
    uint16_t dport;
} __PACK__ pds_flow_key_t;

enum pds_flow_stats_summary_e {
#define _(n, s) FLOW_STATS_##n##_SESSION_COUNT,
    foreach_flow_type_counter
#undef _

    FLOW_STATS_MAX
};

/// \brief flow table statistics summary, for v4 & v6 flows
typedef struct pds_flow_stats_summary_s {
    uint64_t value[FLOW_STATS_MAX];
} __PACK__ pds_flow_stats_summary_t;

#endif    // __INCLUDE_API_PDS_FLOW_HPP__
