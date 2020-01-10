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
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/mapping_impl.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL_STATE - mapping database functionality
/// \ingroup PDS_MAPPING
/// \@{

mapping_impl_state::mapping_impl_state(pds_state *state) {
    p4pd_table_properties_t                   tinfo;
    sdk::table::sdk_table_factory_params_t    mhparams;
    sdk::table::sdk_table_api_params_t        api_params;

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

    bzero(&mhparams, sizeof(mhparams));
    mhparams.table_id = P4TBL_ID_NAT;
    mhparams.entry_trace_en = true;
    nat_tbl_ = sldirectmap::factory(&mhparams);
    SDK_ASSERT(nat_tbl_ != NULL);
    // reserve 0th entry for no xlation
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                          NAT_TX_TBL_RSVD_ENTRY_IDX,
                                          NULL, NULL);
    nat_tbl_->reserve_index(&api_params);

    // create a slab for mapping impl entries
    mapping_impl_slab_ = slab::factory("mapping-impl", PDS_SLAB_ID_MAPPING_IMPL,
                                       sizeof(mapping_impl), 8192, true, true);
    SDK_ASSERT(mapping_impl_slab_!= NULL);
}

mapping_impl_state::~mapping_impl_state() {
    mem_hash::destroy(local_ip_mapping_tbl_);
    mem_hash::destroy(remote_vnic_mapping_rx_tbl_);
    mem_hash::destroy(remote_vnic_mapping_tx_tbl_);
    sldirectmap::destroy(nat_tbl_);
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

void
mapping_dump_cb(sdk_table_api_params_t *params)
{
    int fd = *(int *)(params->cbdata);

    local_ip_mapping_swkey_t *key = (local_ip_mapping_swkey_t *)(params->key);
    local_ip_mapping_appdata_t *data =(local_ip_mapping_appdata_t *)(params->appdata);

    dprintf(fd, "%-7u%-16s%-7u%-8u%-6s\n",
            key->key_metadata_lkp_id,
            ipv4addr2str(*(uint32_t *)key->control_metadata_mapping_lkp_addr),
            data->vpc_id,
            data->xlate_index,
            (data->ip_type == IP_TYPE_OVERLAY) ? "overlay" : "public");
}

sdk_ret_t
mapping_impl_state::mapping_dump(int fd, cmd_args_t *args) {
    sdk_table_api_params_t api_params = { 0 };

    dprintf(fd, "%s\n", std::string(44, '-').c_str());
    dprintf(fd, "%-7s%-16s%-7s%-8s%-6s\n",
            "VnicID", "PrivateIP", "VpcID", "XlateID", "IPType");
    dprintf(fd, "%s\n", std::string(44, '-').c_str());

    if (!args) {
        api_params.itercb = mapping_dump_cb;
        api_params.cbdata = &fd;
        local_ip_mapping_tbl_->iterate(&api_params);
    } else {
        mapping_dump_args_t         *mapping_args = &args->mapping_dump;
        local_ip_mapping_swkey_t    local_ip_mapping_key = { 0 };
        local_ip_mapping_appdata_t  local_ip_mapping_data = { 0 };
        sdk_ret_t                   ret;
        vpc_entry                   *vpc;

        vpc = vpc_db()->find(&mapping_args->key.vpc);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             vpc->hw_id(),
                                             &mapping_args->key.ip_addr,
                                             true);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                       NULL, &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       sdk::table::handle_t::null());
        api_params.cbdata = &fd;

        ret = local_ip_mapping_tbl_->get(&api_params);
        if (ret == SDK_RET_OK) {
            mapping_dump_cb(&api_params);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t    tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_LOCAL_IP_MAPPING, &tinfo);
    stats.table_name = tinfo.tablename;
    local_ip_mapping_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_REMOTE_VNIC_MAPPING_RX, &tinfo);
    stats.table_name = tinfo.tablename;
    remote_vnic_mapping_rx_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_table_properties_get(P4TBL_ID_REMOTE_VNIC_MAPPING_TX, &tinfo);
    stats.table_name = tinfo.tablename;
    remote_vnic_mapping_tx_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
