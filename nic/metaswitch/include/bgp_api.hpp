//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines BGP API
///
//----------------------------------------------------------------------------

#ifndef __BGP_API_HPP__
#define __BGP_API_HPP__

/// \global BGP specification
typedef struct bgp_global_spec_ {
    uint32_t localAsn;
    uint32_t vrfId;
    uint16_t routerId;
} bgp_global_spec_t;
#endif    // __BGP_API_HPP__ 
