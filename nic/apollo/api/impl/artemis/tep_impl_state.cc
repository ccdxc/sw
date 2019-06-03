//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Tunnel EndPoint (TEP) datapath database handling
///
//----------------------------------------------------------------------------


#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/artemis/tep_impl_state.hpp"
#include "gen/p4gen/artemis_txdma/include/artemis_txdma_p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL_STATE - TEP database functionality
/// \ingroup PDS_TEP
/// \@{

tep_impl_state::tep_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;

    // instantiate P4 tables for bookkeeping
    p4pd_global_table_properties_get(P4_ARTEMIS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                     &tinfo);
    // allocate indexer for vnic hw id allocation
    remote_46_tep_idxr_ = indexer::factory(tinfo.tabledepth);
    SDK_ASSERT(remote_46_tep_idxr_ != NULL);
}

tep_impl_state::~tep_impl_state() {
    indexer::destroy(remote_46_tep_idxr_);
}

sdk_ret_t
tep_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
