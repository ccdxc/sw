//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL_STATE - nexthop database functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_impl_state::nexthop_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    p4pd_global_table_properties_get(P4TBL_ID_NEXTHOP, &tinfo);
    // create indexer and reserve system default blackhole/drop nexthop entry
    nh_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(nh_idxr_ != NULL);
}

nexthop_impl_state::~nexthop_impl_state() {
    rte_indexer::destroy(nh_idxr_);
}

/// \@}    // end of PDS_NEXTHOP_IMPL_STATE

}    // namespace impl
}    // namespace api
