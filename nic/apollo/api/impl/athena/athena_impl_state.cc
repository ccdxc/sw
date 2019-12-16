//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline global state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/athena/athena_impl_state.hpp"
#include "gen/p4gen/athena/include/p4pd.h"

namespace api {
namespace impl {

// TODO: table_health_monitor_cb is passed as NULL everywhere here
athena_impl_state::athena_impl_state(pds_state *state) {
    sdk::table::sdk_table_factory_params_t table_params;

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_KEY_NATIVE;
    table_params.entry_trace_en = true;
    key_native_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(key_native_tbl() != NULL);

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_KEY_TUNNELED;
    table_params.entry_trace_en = true;
    key_tunneled_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(key_tunneled_tbl() != NULL);

#if 0
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4I_DROP_STATS;
    table_params.entry_trace_en = true;
    ingress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(ingress_drop_stats_tbl() != NULL);

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4E_DROP_STATS;
    table_params.entry_trace_en = true;
    egress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(egress_drop_stats_tbl() != NULL);
#endif

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_NACL;
    table_params.entry_trace_en = true;
    nacl_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(nacl_tbl() != NULL);
}

athena_impl_state::~athena_impl_state() {
    sltcam::destroy(key_native_tbl());
    sltcam::destroy(key_tunneled_tbl());
    sltcam::destroy(ingress_drop_stats_tbl());
    sltcam::destroy(egress_drop_stats_tbl());
    sltcam::destroy(nacl_tbl());
}

}    // namespace impl
}    // namespace api
