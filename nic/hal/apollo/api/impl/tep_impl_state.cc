/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep_impl_state.cc
 *
 * @brief   This file contains Tunnel EndPoint (TEP)
 *          datapath database handling
 */

#include "nic/hal/apollo/include/api/oci_tep.hpp"
#include "nic/hal/apollo/api/impl/tep_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace impl {

/**
 * @defgroup OCI_TEP_IMPL_STATE - tep database functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    constructor
 */
tep_impl_state::tep_impl_state() {
    p4pd_table_properties_t    tinfo;

    tep_idxr_ = indexer::factory(OCI_MAX_TEP);
    SDK_ASSERT(tep_idxr_ != NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_TEP_TX, &tinfo);
    // TODO: table_health_monitor_cb is passed as NULL here !!
    tep_tx_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_TEP_TX,
                                     tinfo.tabledepth,
                                     tinfo.actiondata_struct_size,
                                     false, true, NULL);
    SDK_ASSERT(tep_tx_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
tep_impl_state::~tep_impl_state() {
    indexer::destroy(tep_idxr_);
    directmap::destroy(tep_tx_tbl_);
}

/** @} */    // end of OCI_TEP_IMPL_STATE

}    // namespace impl

