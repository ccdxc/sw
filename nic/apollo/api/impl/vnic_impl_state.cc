/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl_state.cc
 *
 * @brief   This file contains vnic datapath database handling
 */

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/impl/vnic_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

// TODO: HACK !!!, remove
void table_health_monitor_cb(uint32_t table_id,
                             char *name,
                             table_health_state_t curr_state,
                             uint32_t capacity,
                             uint32_t usage,
                             table_health_state_t *new_state) {
}

namespace api {
namespace impl {

/**
 * @defgroup PDS_VNIC_IMPL_STATE - vnic database functionality
 * @ingroup PDS_VNIC
 * @{
 */

/**
 * @brief    constructor
 */
vnic_impl_state::vnic_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo, oflow_tinfo;

    vnic_idxr_ = indexer::factory(PDS_MAX_VNIC);
    SDK_ASSERT(vnic_idxr_ != NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX, &tinfo);
    local_vnic_by_vlan_tx_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_vlan_tx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, &tinfo);
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                              &oflow_tinfo);
    local_vnic_by_slot_rx_tbl_ =
        sdk_hash::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                          P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                          tinfo.tabledepth,
                          oflow_tinfo.tabledepth,
                          tinfo.key_struct_size,
                          tinfo.actiondata_struct_size,
                          static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                          true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_slot_rx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO, &tinfo);
    egress_local_vnic_info_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_EGRESS_LOCAL_VNIC_INFO,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(egress_local_vnic_info_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
vnic_impl_state::~vnic_impl_state() {
    indexer::destroy(vnic_idxr_);
    directmap::destroy(local_vnic_by_vlan_tx_tbl_);
    sdk_hash::destroy(local_vnic_by_slot_rx_tbl_);
    directmap::destroy(egress_local_vnic_info_tbl_);
}

/**
 * @brief    API to initiate transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl_state::table_transaction_begin(void) {
    //vnic_idxr_->txn_start();
    //local_vnic_by_vlan_tx_tbl_->txn_start();
    //local_vnic_by_slot_rx_tbl_->txn_start();
    //egress_local_vnic_info_tbl_->txn_start();
    return SDK_RET_OK;
}

/**
 * @brief    API to end transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl_state::table_transaction_end(void) {
    //vnic_idxr_->txn_end();
    //local_vnic_by_vlan_tx_tbl_->txn_end();
    //local_vnic_by_slot_rx_tbl_->txn_end();
    //egress_local_vnic_info_tbl_->txn_end();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_VNIC_IMPL_STATE

}    // namespace impl
}    // namespace api
