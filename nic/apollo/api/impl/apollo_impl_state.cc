/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    apollo_impl_state.cc
 *
 * @brief   pipeline global state maintenance
 */
#include "nic/apollo/include/api/oci_tep.hpp"
#include "nic/apollo/api/impl/apollo_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace impl {

/**
 * @defgroup OCI_APOLLO_IMPL_STATE - tep database functionality
 * @ingroup OCI_APOLLO
 * @{
 */

/**
 * @brief    constructor
 */
apollo_impl_state::apollo_impl_state(oci_state *state) {
    p4pd_table_properties_t    tinfo;

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_KEY_NATIVE, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    key_native_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_KEY_NATIVE,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(key_native_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_KEY_TUNNELED, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    key_tunneled_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_KEY_TUNNELED,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(key_tunneled_tbl_ != NULL);

    ingress_drop_stats_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_P4I_DROP_STATS,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(ingress_drop_stats_tbl_ != NULL);

    egress_drop_stats_tbl_ =
        tcam::factory(tinfo.tablename, P4TBL_ID_P4E_DROP_STATS,
                      tinfo.tabledepth, tinfo.key_struct_size,
                      tinfo.actiondata_struct_size,
                      false, true, NULL);
    SDK_ASSERT(egress_drop_stats_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
apollo_impl_state::~apollo_impl_state() {
    tcam::destroy(key_native_tbl_);
    tcam::destroy(key_tunneled_tbl_);
    tcam::destroy(ingress_drop_stats_tbl_);
    tcam::destroy(egress_drop_stats_tbl_);
}

/** @} */    // end of OCI_APOLLO_IMPL_STATE

}    // namespace impl
