//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of device object
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/apulu/device_impl.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_DEVICE_IMPL - device entry datapath implementation
/// \ingroup PDS_DEVICE
/// \@{

// as there is no state in this impl, single instance is good enough
device_impl    g_device_impl;

device_impl *
device_impl::factory(pds_device_spec_t *pds_device) {
    return &g_device_impl;
}

void
device_impl::destroy(device_impl *impl) {
    return;
}

void
device_impl::fill_spec_(pds_device_spec_t *spec) {
}

uint32_t
device_impl::fill_ing_drop_stats_(pds_device_drop_stats_t *ing_drop_stats) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = { 0 };
    p4i_drop_stats_swkey_mask_t key_mask = { 0 };
    p4i_drop_stats_actiondata_t data = { 0 };
    const char idrop[][PDS_MAX_DROP_NAME_LEN] = {
        "drop_src_mac_zero",
        "drop_src_mac_mismatch",
        "drop_vnic_tx_miss",
        "drop_vnic_rx_miss",
        "drop_src_dst_check_fail",
        "drop_flow_hit",
        "drop_tep_rx_dst_ip_mismatch",
        "drop_rvpath_src_ip_mismatch",
        "drop_rvpath_vpc_mismatch",
        "drop_nacl"
    };

    SDK_ASSERT(sizeof(idrop)/sizeof(idrop[0]) == (P4I_DROP_REASON_MAX + 1));
    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; ++i) {
        if (p4pd_global_entry_read(P4TBL_ID_P4I_DROP_STATS, i,
                                   &key, &key_mask, &data) == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts));
            ing_drop_stats[i].count = pkts;
            strcpy(ing_drop_stats[i].name, idrop[i]);
        }
    }
    return P4I_DROP_REASON_MAX + 1;
}

uint32_t
device_impl::fill_egr_drop_stats_(pds_device_drop_stats_t *egr_drop_stats) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4e_drop_stats_swkey_t key = { 0 };
    p4e_drop_stats_swkey_mask_t key_mask = { 0 };
    p4e_drop_stats_actiondata_t data = { 0 };
    const char edrop[][PDS_MAX_DROP_NAME_LEN] = {
        "drop_nexthop_invalid"
    };

    SDK_ASSERT(sizeof(edrop)/sizeof(edrop[0]) == (P4E_DROP_REASON_MAX + 1));
    for (uint32_t i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; ++i) {
        if (p4pd_global_entry_read(P4TBL_ID_P4E_DROP_STATS, i,
                                   &key, &key_mask, &data) == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4e_drop_stats_p4e_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4e_drop_stats_p4e_drop_stats.drop_stats_pkts));
            egr_drop_stats[i].count = pkts;
            strcpy(egr_drop_stats[i].name, edrop[i]);
        }
    }
    return P4E_DROP_REASON_MAX + 1;
}

sdk_ret_t
device_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    pds_device_info_t *dinfo = (pds_device_info_t *)info;

    (void)api_obj;
    (void)key;
    fill_spec_(&dinfo->spec);
    // TODO: rename ing_drop_stats_count and egr_drop_stats_count to
    //       num_ing_drop_stats and num_egr_drop_stats respectively
    dinfo->stats.ing_drop_stats_count =
        fill_ing_drop_stats_(&dinfo->stats.ing_drop_stats[0]);
    dinfo->stats.egr_drop_stats_count =
        fill_egr_drop_stats_(&dinfo->stats.egr_drop_stats[0]);
    return sdk::SDK_RET_OK;
}

sdk_ret_t
device_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    device_entry *device;
    ip_addr_t ip_addr;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        device = (device_entry *)api_obj;
        ip_addr = device->ip_addr();
        PDS_TRACE_DEBUG("Activating device IP %s, MAC %s as table constants",
                        ipaddr2str(&ip_addr), macaddr2str(device->mac()));
        break;

    case API_OP_DELETE:
        PDS_TRACE_DEBUG("Cleaning up device config");
        break;

    default:
        return SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
