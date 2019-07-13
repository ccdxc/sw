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
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/nexthop_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL_STATE - tep database functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_impl_state::nexthop_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    p4pd_global_table_properties_get(P4TBL_ID_NEXTHOP, &tinfo);
    nh_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NEXTHOP,
                                 tinfo.tabledepth,
                                 tinfo.actiondata_struct_size,
                                 false, true, NULL);
    SDK_ASSERT(nh_tbl_ != NULL);
    // reserve system default blackhole/drop nexthop entry
    nh_tbl_->reserve_index(PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID);
}

nexthop_impl_state::~nexthop_impl_state() {
    directmap::destroy(nh_tbl_);
}

sdk_ret_t
nexthop_impl_state::table_transaction_begin(void) {
    //nh_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl_state::table_transaction_end(void) {
    //nh_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_IMPL_STATE

}    // namespace impl
}    // namespace api
