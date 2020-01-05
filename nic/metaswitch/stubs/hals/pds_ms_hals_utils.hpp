//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// HALS Utils for Metaswitch joins and Stores
//---------------------------------------------------------------

#ifndef __PDS_MS_HALS_UTILS_HPP__
#define __PDS_MS_HALS_UTILS_HPP__

#include <hals_fte.hpp>
#include <hals_ropi_slave_join.hpp>
#include <hals_route.hpp>

namespace pds_ms {

static inline hals::RopiSlaveJoin* get_hals_ropi_join() {
    // TODO: Use get_partner_entity_index() to get the correct join for the VRF
    auto& ropi_set = hals::Fte::get().get_ropi_joins();
    if (unlikely(ropi_set.size()!=1)) {
        SDK_TRACE_ERR("Unexpected number of ROPI joins %d", 
                      ropi_set.size());
        return nullptr;
    }
    return *(ropi_set.begin());
}

} // End namespace

#endif
