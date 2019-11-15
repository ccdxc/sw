//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policer datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/policer_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_POLICER_IMPL_STATE - policer database functionality
/// \ingroup PDS_POLICER
/// \@{

policer_impl_state::policer_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;

    p4pd_global_table_properties_get(P4TBL_ID_VNIC_POLICER_TX, &tinfo);
    tx_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(tx_idxr_ != NULL);

    p4pd_global_table_properties_get(P4TBL_ID_VNIC_POLICER_RX, &tinfo);
    rx_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(rx_idxr_ != NULL);
}

policer_impl_state::~policer_impl_state() {
    rte_indexer::destroy(tx_idxr_);
    rte_indexer::destroy(rx_idxr_);
}

/// \@}    // end of PDS_POLICER_IMPL_STATE

}    // namespace impl
}    // namespace api
