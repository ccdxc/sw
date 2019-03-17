//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mapping datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "gen/p4gen/apollo/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping database functionality
/// \ingroup PDS_MAPPING
/// \@{

mapping_impl_state::mapping_impl_state(pds_state *state) {
    p4pd_table_properties_t       tinfo;
    sdk_table_factory_params_t    mhparams;

    // instantiate P4 tables for bookkeeping
    bzero(&mhparams, sizeof(mhparams));
    mhparams.max_recircs = 8;
    //mhparams.health_monitor_func = NULL;

    // local IP Mapping table
    mhparams.table_id = P4TBL_ID_LOCAL_IP_MAPPING;
    mhparams.num_hints = P4_LOCAL_IP_MAPPING_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    local_ip_mapping_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(local_ip_mapping_tbl_ != NULL);

    // remote VNIC Mapping RX table
    mhparams.table_id = P4TBL_ID_REMOTE_VNIC_MAPPING_RX;
    mhparams.num_hints = P4_REMOTE_VNIC_MAPPING_RX_NUM_HINTS_PER_ENTRY;
    mhparams.key2str = NULL;
    mhparams.appdata2str = NULL;
    remote_vnic_mapping_rx_tbl_ = mem_hash::factory(&mhparams);
    SDK_ASSERT(remote_vnic_mapping_rx_tbl_ != NULL);

    // remote VNIC Mapping RX table
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
    // reserve 0th entry for no xlation
    nat_tbl_->reserve_index(NAT_TX_TBL_RSVD_ENTRY_IDX);

    // create a slab for mapping impl entries
    mapping_impl_slab_ = slab::factory("mapping_impl", PDS_SLAB_ID_MAPPING_IMPL,
                                       sizeof(mapping_impl), 8192, true, true);
    SDK_ASSERT(mapping_impl_slab_!= NULL);
}

mapping_impl_state::~mapping_impl_state() {
    mem_hash::destroy(local_ip_mapping_tbl_);
    mem_hash::destroy(remote_vnic_mapping_rx_tbl_);
    mem_hash::destroy(remote_vnic_mapping_tx_tbl_);
    directmap::destroy(nat_tbl_);
    slab::destroy(mapping_impl_slab_);
}

mapping_impl *
mapping_impl_state::alloc(void) {
    return ((mapping_impl *)mapping_impl_slab_->alloc());
}

void
mapping_impl_state::free(mapping_impl *impl) {
    mapping_impl_slab_->free(impl);
}

sdk_ret_t
mapping_impl_state::table_transaction_begin(void) {
    local_ip_mapping_tbl_->txn_start();
    remote_vnic_mapping_rx_tbl_->txn_start();
    remote_vnic_mapping_tx_tbl_->txn_start();
    //nat_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::table_transaction_end(void) {
    local_ip_mapping_tbl_->txn_end();
    remote_vnic_mapping_rx_tbl_->txn_end();
    remote_vnic_mapping_tx_tbl_->txn_end();
    //nat_tbl_->txn_end();
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
