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
#include "nic/apollo/api/impl/apulu/policer_impl.hpp"
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

policer_impl *
policer_impl_state::alloc(void) {
    return (policer_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_POLICER_IMPL,
                                      sizeof(policer_impl));
}

void
policer_impl_state::free(policer_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_POLICER_IMPL, impl);
}

/// \@}    // end of PDS_POLICER_IMPL_STATE

}    // namespace impl
}    // namespace api
