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

/// \brief flow key
typedef struct pds_flow_key_s {
    uint32_t lookup_id;
    ip_addr_t src_ip;
    ip_addr_t dst_ip;
    uint16_t proto;
    uint16_t sport;
    uint16_t dport;
} __PACK__ pds_flow_key_t;

#endif    // __INCLUDE_API_PDS_FLOW_HPP__
