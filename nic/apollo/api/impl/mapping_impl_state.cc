/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl_state.cc
 *
 * @brief   This file contains mapping datapath database handling
 */
#include "nic/apollo/include/api/oci_mapping.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/p4/include/defines.h"

#include "nic/sdk/lib/table/memhash/mem_hash.hpp"

using sdk::table::mem_hash_factory_params_t;

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL_STATE - mapping database functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    constructor
 */
mapping_impl_state::mapping_impl_state(oci_state *state) {
    p4pd_table_properties_t tinfo;
    mem_hash_factory_params_t mhparams;

    /**< instantiate P4 tables for bookkeeping */
    bzero(&mhparams, sizeof(mhparams));
    mhparams.max_recircs = 8;
    mhparams.health_monitor_func = NULL;

    // Local IP Mapping table
    mhparams.table_id = P4TBL_ID_LOCAL_IP_MAPPING;
    mhparams.num_hints = P4_LOCAL_IP_MAPPING_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    local_ip_mapping_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(local_ip_mapping_tbl_ != NULL);

    // Remote VNIC Mapping RX table
    mhparams.table_id = P4TBL_ID_REMOTE_VNIC_MAPPING_RX;
    mhparams.num_hints = P4_REMOTE_VNIC_MAPPING_RX_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    remote_vnic_mapping_rx_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(remote_vnic_mapping_rx_tbl_ != NULL);

    // Remote VNIC Mapping RX table
    mhparams.table_id = P4TBL_ID_REMOTE_VNIC_MAPPING_TX;
    mhparams.num_hints = P4_REMOTE_VNIC_MAPPING_TX_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    remote_vnic_mapping_tx_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(remote_vnic_mapping_tx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_NAT, &tinfo);
    nat_tbl_ = directmap::factory(tinfo.tablename, P4TBL_ID_NAT,
                                  tinfo.tabledepth,
                                  tinfo.actiondata_struct_size,
                                  false, true, NULL);
    SDK_ASSERT(nat_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
mapping_impl_state::~mapping_impl_state() {
    mem_hash::destroy(local_ip_mapping_tbl_);
    mem_hash::destroy(remote_vnic_mapping_rx_tbl_);
    mem_hash::destroy(remote_vnic_mapping_tx_tbl_);
}

/** @} */    // end of OCI_MAPPING_IMPL_STATE

}    // namespace impl
