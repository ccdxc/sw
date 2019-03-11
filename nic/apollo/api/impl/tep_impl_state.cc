/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep_impl_state.cc
 *
 * @brief   This file contains Tunnel EndPoint (TEP)
 *          datapath database handling
 */
#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/api/impl/tep_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_TEP_IMPL_STATE - tep database functionality
 * @ingroup PDS_TEP
 * @{
 */

/**
 * @brief    constructor
 */
tep_impl_state::tep_impl_state(pds_state *state) {
    p4pd_table_properties_t    tinfo;

    tep_idxr_ = indexer::factory(PDS_MAX_TEP);
    SDK_ASSERT(tep_idxr_ != NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_TEP_TX, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    tep_tx_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_TEP_TX,
                                     tinfo.tabledepth,
                                     tinfo.actiondata_struct_size,
                                     false, true, NULL);
    SDK_ASSERT(tep_tx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_NEXTHOP_TX, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    nh_tx_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NEXTHOP_TX,
                                    tinfo.tabledepth,
                                    tinfo.actiondata_struct_size,
                                    false, true, NULL);
    SDK_ASSERT(nh_tx_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
tep_impl_state::~tep_impl_state() {
    indexer::destroy(tep_idxr_);
    directmap::destroy(tep_tx_tbl_);
    directmap::destroy(nh_tx_tbl_);
}

/**
 * @brief    API to initiate transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl_state::table_transaction_begin(void) {
    //tep_idxr_->txn_start();
    //tep_tx_tbl_->txn_start();
    //nh_tx_tbl_->txn_start();
    return SDK_RET_OK;
}

/**
 * @brief    API to end transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl_state::table_transaction_end(void) {
    //tep_idxr_->txn_end();
    //tep_tx_tbl_->txn_end();
    //nh_tx_tbl_->txn_end();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_TEP_IMPL_STATE

}    // namespace impl
}    // namespace api
