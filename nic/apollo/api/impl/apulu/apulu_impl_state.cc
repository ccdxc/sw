//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline global state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_APULU_IMPL_STATE - apulu database functionality
/// \ingroup PDS_APULU
/// \@{

apulu_impl_state::apulu_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t table_params;

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4I_DROP_STATS;
    table_params.entry_trace_en = false;
    ingress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(ingress_drop_stats_tbl() != NULL);

    memset(&table_params, 0, sizeof(table_params));
    table_params.table_id = P4TBL_ID_P4E_DROP_STATS;
    table_params.entry_trace_en = false;
    egress_drop_stats_tbl_ = sltcam::factory(&table_params);
    SDK_ASSERT(egress_drop_stats_tbl() != NULL);

    // allocate indexer for NACL table
    p4pd_global_table_properties_get(P4TBL_ID_NACL, &tinfo);
    nacl_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, false);
    SDK_ASSERT(nacl_idxr_ != NULL);
    // pre-allocate indices for the reserved portion of NACL table
    // and manage it separately
    nacl_idxr_->alloc_block((uint32_t)PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MIN,
                    PDS_IMPL_NACL_BLOCK_GENERIC_MIN - PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MIN,
                    false);

    // bookkeeping for CoPP table
    p4pd_global_table_properties_get(P4TBL_ID_COPP, &tinfo);
    copp_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, true);
    SDK_ASSERT(copp_idxr_ != NULL);

    // NAT table bookkeeping (reserve 0th entry for no xlation)
    // NOTE: 1st half of the table is used for config and 2nd half by VPP
    //       for dynamic bindings
    p4pd_table_properties_get(P4TBL_ID_NAT, &tinfo);
    nat_idxr_ = rte_indexer::factory(tinfo.tabledepth >> 1, true, true);
    SDK_ASSERT(nat_idxr_ != NULL);

    // DNAT table bookkeeping
    p4pd_table_properties_get(P4_P4PLUS_TXDMA_TBL_ID_DNAT, &tinfo);
    dnat_idxr_ = rte_indexer::factory(tinfo.tabledepth, true, false);
    SDK_ASSERT(dnat_idxr_ != NULL);
}

apulu_impl_state::~apulu_impl_state() {
    sltcam::destroy(ingress_drop_stats_tbl_);
    sltcam::destroy(egress_drop_stats_tbl_);
    rte_indexer::destroy(nacl_idxr_);
    rte_indexer::destroy(copp_idxr_);
    rte_indexer::destroy(nat_idxr_);
    rte_indexer::destroy(dnat_idxr_);
}

sdk_ret_t
apulu_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl_state::nacl_dump(int fd) {
    nacl_swkey_t key;
    p4pd_error_t p4pd_ret;
    nacl_swkey_mask_t mask;
    nacl_actiondata_t data;

    for (uint32_t i = 0; i < nacl_idxr_->size(); i++) {
        if (nacl_idxr_->is_index_allocated(i)) {
            p4pd_ret = p4pd_entry_read(P4TBL_ID_NACL, i, &key, &mask, &data);
            if (p4pd_ret == P4PD_SUCCESS) {
                dprintf(fd, "%-3u. ", i);
                if (mask.control_metadata_lif_type_mask) {
                    dprintf(fd, "lif type - %u",
                            key.control_metadata_lif_type);
                } else {
                    dprintf(fd, "lif type - *");
                }
                if (mask.capri_intrinsic_lif_mask) {
                    dprintf(fd, ", lif - %u", key.capri_intrinsic_lif);
                } else {
                    dprintf(fd, ", lif - *");
                }
                if (mask.control_metadata_learn_enabled_mask) {
                    dprintf(fd, ", learn en - %u",
                            key.control_metadata_learn_enabled);
                } else {
                    dprintf(fd, ", learn en - *");
                }
                if (mask.control_metadata_flow_miss_mask) {
                    dprintf(fd, ", flow miss - %u",
                            key.control_metadata_flow_miss);
                } else {
                    dprintf(fd, ", flow miss - *");
                }
                if (mask.control_metadata_rx_packet_mask) {
                    dprintf(fd, ", rx - %u", key.control_metadata_rx_packet);
                } else {
                    dprintf(fd, ", rx - *");
                }
                if (mask.control_metadata_tunneled_packet_mask) {
                    dprintf(fd, ", tunneled pkt - %u",
                            key.control_metadata_tunneled_packet);
                } else {
                    dprintf(fd, ", tunneled pkt - *");
                }
                if (mask.key_metadata_ktype_mask) {
                    if (key.key_metadata_ktype == KEY_TYPE_IPV4) {
                        dprintf(fd, ", key type - v4");
                    } else if (key.key_metadata_ktype == KEY_TYPE_IPV6) {
                        dprintf(fd, ", key type - v6");
                    } else if (key.key_metadata_ktype == KEY_TYPE_MAC) {
                        dprintf(fd, ", key type - mac");
                    }
                } else {
                    dprintf(fd, ", key type - *");
                }
                if (mask.arm_to_p4i_nexthop_valid_mask) {
                    dprintf(fd, ", arm -> p4 nh valid - %u",
                            key.arm_to_p4i_nexthop_valid);
                } else {
                    dprintf(fd, ", arm -> p4 nh valid - *");
                }
                if (mask.control_metadata_local_mapping_miss_mask) {
                    dprintf(fd, ", local_mapping_miss - %u",
                            key.control_metadata_local_mapping_miss);
                } else {
                    dprintf(fd, ", local_mapping_miss - *");
                }
                if (memcmp(mask.ethernet_1_dstAddr_mask, &g_zero_mac,
                           sizeof(mask.ethernet_1_dstAddr_mask))) {
                    dprintf(fd, ", dmac - %s",
                            macaddr2str(key.ethernet_1_dstAddr));
                } else {
                    dprintf(fd, ", dmac - *");
                }
                if (mask.key_metadata_proto_mask) {
                    dprintf(fd, ", proto - %u", key.key_metadata_proto);
                } else {
                    dprintf(fd, ", proto - *");
                }
                if (memcmp(mask.key_metadata_src_mask, &g_zero_ip.addr,
                           sizeof(mask.key_metadata_src_mask))) {
                   // TODO: print DIP based on key type
                } else {
                    dprintf(fd, ", sip - *");
                }
                if (memcmp(mask.key_metadata_dst_mask, &g_zero_ip.addr,
                           sizeof(mask.key_metadata_dst_mask))) {
                   // TODO: print DIP based on key type
                } else {
                    dprintf(fd, ", dip - *");
                }
                if (mask.key_metadata_sport_mask) {
                    dprintf(fd, ", sport - %u", key.key_metadata_sport);
                } else {
                    dprintf(fd, ", sport - *");
                }
                if (mask.key_metadata_dport_mask) {
                    dprintf(fd, ", dport - %u", key.key_metadata_sport);
                } else {
                    dprintf(fd, ", dport - *");
                }
                if (mask.vnic_metadata_vnic_id_mask) {
                    dprintf(fd, ", vnic id - %u", key.vnic_metadata_vnic_id);
                } else {
                    dprintf(fd, ", vnic id - *");
                }
                dprintf(fd, "\n");
            } else {
                PDS_TRACE_ERR("Failed to read NACL entry at idx %u", i);
            }
        }
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
