//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_GROUP_IMPL_STATE - nexthop group database
///           functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_group_impl_state::nexthop_group_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    p4pd_global_table_properties_get(P4TBL_ID_ECMP, &tinfo);
    nhgroup_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(nhgroup_idxr_ != NULL);
}

nexthop_group_impl_state::~nexthop_group_impl_state() {
    rte_indexer::destroy(nhgroup_idxr_);
}

nexthop_group_impl *
nexthop_group_impl_state::alloc(void) {
    return (nexthop_group_impl *)
               SDK_CALLOC(SDK_MEM_ALLOC_PDS_NEXTHOP_GROUP_IMPL,
                          sizeof(nexthop_group_impl));
}

void
nexthop_group_impl_state::free(nexthop_group_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_NEXTHOP_GROUP_IMPL, impl);
}

/// \@}    // end of PDS_NEXTHOP_GROUP_IMPL_STATE

}    // namespace impl
}    // namespace api
