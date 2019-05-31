//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vnic datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/impl/artemis/vnic_impl_state.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "gen/p4gen/artemis_rxdma/include/artemis_rxdma_p4pd.h"
#include "gen/p4gen/artemis_txdma/include/artemis_txdma_p4pd.h"

namespace api {
namespace impl {

/**
 * @defgroup PDS_VNIC_IMPL_STATE - vnic database functionality
 * @ingroup PDS_VNIC
 * @{
 */

vnic_impl_state::vnic_impl_state(pds_state *state) {
    p4pd_table_properties_t       tinfo, oflow_tinfo;
    sdk_table_factory_params_t    table_params;

    // allocate indexer for vnic hw id allocation
    vnic_idxr_ = indexer::factory(PDS_MAX_VNIC);
    SDK_ASSERT(vnic_idxr_ != NULL);

    // VNIC_MAPPING tcam table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_VNIC_MAPPING;
    table_params.entry_trace_en = true;
    vnic_mapping_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(vnic_mapping_tbl_ != NULL);
}

vnic_impl_state::~vnic_impl_state() {
    indexer::destroy(vnic_idxr_);
    sltcam::destroy(vnic_mapping_tbl_);
}

sdk_ret_t
vnic_impl_state::table_transaction_begin(void) {
    //vnic_idxr_->txn_start();
    vnic_mapping_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::table_transaction_end(void) {
    //vnic_idxr_->txn_end();
    vnic_mapping_tbl_->txn_end();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_VNIC_IMPL_STATE

}    // namespace impl
}    // namespace api
