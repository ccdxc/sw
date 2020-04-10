//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Typedefs for Metaswitch types
//---------------------------------------------------------------

#ifndef __PDS_MS_MS_DEFS_HPP__
#define __PDS_MS_MS_DEFS_HPP__

#include <nbase.h>
#include "nic/apollo/core/trace.hpp"

namespace pds_ms {
using ms_ps_id_t = uint32_t; // Pathset ID
using ms_vrf_id_t = uint32_t;
using ms_ifindex_t = NBB_ULONG;
using ms_bd_id_t = NBB_ULONG;
using ms_hw_tbl_id_t = uint32_t; // Used as DP correlator in MS

#define PDS_MS_ECMP_INVALID_INDEX 0

}

#endif
