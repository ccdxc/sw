//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif implementation state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/apollo/api/impl/lif_impl_state.hpp"

/// \defgroup PDS_LIF_IMPL_STATE - lif state functionality
/// \ingroup PDS_LIF
/// \@{

namespace api {
namespace impl {

lif_impl_state::lif_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    p4pd_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE,
                              &tinfo);
    tx_rate_limiter_tbl_ =
        directmap::factory(tinfo.tablename,
                           P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, NULL);
    SDK_ASSERT(tx_rate_limiter_tbl_ != NULL);
}

lif_impl_state::~lif_impl_state() {
    directmap::destroy(tx_rate_limiter_tbl_);
}

/// \@}

}    // namespace impl
}    // namespace api
