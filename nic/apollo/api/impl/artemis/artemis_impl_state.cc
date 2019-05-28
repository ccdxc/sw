//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline global state maintenance
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl_state.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ARTEMIS_IMPL_STATE - tep database functionality
/// \ingroup PDS_ARTEMIS
/// \@{

// constructor
artemis_impl_state::artemis_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    // instantiate P4 tables for bookkeeping
    p4pd_table_properties_get(P4TBL_ID_KEY_NATIVE, &tinfo);
    key_native_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_KEY_NATIVE,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(key_native_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_KEY_TUNNELED, &tinfo);
    key_tunneled_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_KEY_TUNNELED,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(key_tunneled_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_KEY_TUNNELED2, &tinfo);
    key_tunneled2_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_KEY_TUNNELED2,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(key_tunneled2_tbl_ != NULL);

    // instantiate ingress drop stats table
    p4pd_table_properties_get(P4TBL_ID_P4I_DROP_STATS, &tinfo);
    ingress_drop_stats_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_P4I_DROP_STATS,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, false, NULL);
    SDK_ASSERT(ingress_drop_stats_tbl_ != NULL);

    // instantiate egress drop stats table
    p4pd_table_properties_get(P4TBL_ID_P4E_DROP_STATS, &tinfo);
    egress_drop_stats_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_P4E_DROP_STATS,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, false, NULL);
    SDK_ASSERT(egress_drop_stats_tbl_ != NULL);

    // instantiate NACL table
    p4pd_table_properties_get(P4TBL_ID_NACL, &tinfo);
    nacl_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_NACL, tinfo.tabledepth,
                      tinfo.key_struct_size, tinfo.actiondata_struct_size,
                      false, true, NULL);
     SDK_ASSERT(nacl_tbl_ != NULL);

    // instantiate NAT table
    p4pd_table_properties_get(P4TBL_ID_NAT, &tinfo);
    nat_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NAT,
                                  tinfo.tabledepth,
                                  tinfo.actiondata_struct_size,
                                  false, false, NULL);
    SDK_ASSERT(nat_tbl_ != NULL);
    // reserve 0th entry for no xlation
    nat_tbl_->reserve_index(PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX);

    p4pd_table_properties_get(P4TBL_ID_NEXTHOP, &tinfo);
    nh_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NEXTHOP,
                                 tinfo.tabledepth,
                                 tinfo.actiondata_struct_size,
                                 false, true, NULL);
    nh_tbl_->reserve_index(PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID);
    SDK_ASSERT(nh_tbl_ != NULL);
}

// destructor
artemis_impl_state::~artemis_impl_state() {
    tcam::destroy(key_native_tbl_);
    tcam::destroy(key_tunneled_tbl_);
    tcam::destroy(key_tunneled2_tbl_);
    tcam::destroy(ingress_drop_stats_tbl_);
    tcam::destroy(egress_drop_stats_tbl_);
    tcam::destroy(nacl_tbl_);
    directmap::destroy(nat_tbl_);
    directmap::destroy(nh_tbl_);
}

sdk_ret_t
artemis_impl_state::table_transaction_begin(void) {
    //nat_tbl_->txn_start();
    //nh_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl_state::table_transaction_end(void) {
    //nat_tbl_->txn_end();
    //nh_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}    // end of PDS_ARTEMIS_IMPL_STATE

}    // namespace impl
}    // namespace api
