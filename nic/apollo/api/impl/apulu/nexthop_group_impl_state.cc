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

    p4pd_global_table_properties_get(P4TBL_OVERLAY_ID_NEXTHOP_GROUP, &tinfo);
    overlay_nh_group_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_OVERLAY_NEXTHOP_GROUP,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, NULL);
    SDK_ASSERT(overlay_nh_group_tbl_ != NULL);

    p4pd_global_table_properties_get(P4TBL_OVERLAY_ID_NEXTHOP_GROUP, &tinfo);
    underlay_nh_group_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_UNDERLAY_NEXTHOP_GROUP,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, NULL);
    SDK_ASSERT(underlay_nh_group_tbl_ != NULL);
}

nexthop_group_impl_state::~nexthop_group_impl_state() {
    directmap::destroy(overlay_nh_group_tbl_);
    directmap::destroy(underlay_nh_group_tbl_);
}

sdk_ret_t
nexthop_group_impl_state::table_transaction_begin(void) {
    //overlay_nh_group_tbl_->txn_start();
    //underlay_nh_group_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl_state::table_transaction_end(void) {
    //overlay_nh_group_tbl_->txn_end();
    //underlay_nh_group_tbl_->txn_stop();
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_GROUP_IMPL_STATE

}    // namespace impl
}    // namespace api
