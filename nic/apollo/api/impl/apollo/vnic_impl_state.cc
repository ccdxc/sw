//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// VNIC datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/vnic_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL_STATE - VNIC database functionality
/// \ingroup PDS_VNIC
/// @{

vnic_impl_state::vnic_impl_state(pds_state *state) {
    p4pd_table_properties_t       tinfo, oflow_tinfo;
    sdk_table_factory_params_t    table_params;
    sdk_table_factory_params_t    slhparams;

    // allocate indexer for vnic hw id allocation
    vnic_idxr_ = indexer::factory(PDS_MAX_VNIC);
    SDK_ASSERT(vnic_idxr_ != NULL);

    // LOCAL_VNIC_BY_VLAN_TX tcam table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX;
    table_params.entry_trace_en = true;
    local_vnic_by_vlan_tx_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(local_vnic_by_vlan_tx_tbl_ != NULL);

    // LOCAL_VNIC_BY_SLOT_RX hash + otcam table
    slhparams.table_id = P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX;
    slhparams.key2str = NULL;
    slhparams.appdata2str = NULL;
    local_vnic_by_slot_rx_tbl_ = slhash::factory(&slhparams);
    SDK_ASSERT(local_vnic_by_slot_rx_tbl_ != NULL);

    // EGRESS_LOCAL_VNIC_INFO index table
    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_EGRESS_LOCAL_VNIC_INFO;
    table_params.entry_trace_en = true;
    egress_local_vnic_info_tbl_ = sldirectmap::factory(&table_params);
    SDK_ASSERT(egress_local_vnic_info_tbl_ != NULL);
}

vnic_impl_state::~vnic_impl_state() {
    indexer::destroy(vnic_idxr_);
    sltcam::destroy(local_vnic_by_vlan_tx_tbl_);
    slhash::destroy(local_vnic_by_slot_rx_tbl_);
    sldirectmap::destroy(egress_local_vnic_info_tbl_);
}

sdk_ret_t
vnic_impl_state::table_transaction_begin(void) {
    //vnic_idxr_->txn_start();
    local_vnic_by_vlan_tx_tbl_->txn_start();
    local_vnic_by_slot_rx_tbl_->txn_start();
    //egress_local_vnic_info_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl_state::table_transaction_end(void) {
    //vnic_idxr_->txn_end();
    local_vnic_by_vlan_tx_tbl_->txn_end();
    local_vnic_by_slot_rx_tbl_->txn_end();
    //egress_local_vnic_info_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
