//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of device
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/apollo/device_impl.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "gen/p4gen/apollo/include/p4pd.h"

namespace api {
namespace impl {

#define tep_mpls_udp_action       action_u.tep_mpls_udp_tep
#define tep_ipv4_vxlan_action     action_u.tep_ipv4_vxlan_tep
#define tep_ipv6_vxlan_action     action_u.tep_ipv6_vxlan_tep
#define nh_action                 action_u.nexthop_nexthop_info

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
    uint64_t val;

    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                                              &val);
    if (val) {
        spec->device_ip_addr.af = IP_AF_IPV4;
        spec->device_ip_addr.addr.v4_addr = (ipv4_addr_t)val;
    }
    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_TEP, &val);
    MAC_UINT64_TO_ADDR(spec->device_mac_addr, val);
}

uint32_t
device_impl::fill_ing_drop_stats_(pds_device_drop_stats_t *ing_drop_stats) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = { 0 };
    p4i_drop_stats_swkey_mask_t key_mask = { 0 };
    p4i_drop_stats_actiondata_t data = { 0 };
    const char idrop[][PDS_MAX_DROP_NAME_LEN] = {
        "drop_src_mac_zero",           "drop_src_mac_mismatch",
        "drop_vnic_tx_miss",           "drop_vnic_rx_miss",
        "drop_src_dst_check_fail",     "drop_flow_hit",
        "drop_tep_rx_dst_ip_mismatch", "drop_rvpath_src_ip_mismatch",
        "drop_rvpath_vpc_mismatch",    "drop_nacl"};

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
    const char edrop[][PDS_MAX_DROP_NAME_LEN] = {"drop_nexthop_invalid"};

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
    dinfo->stats.ing_drop_stats_count =
        fill_ing_drop_stats_(&dinfo->stats.ing_drop_stats[0]);
    dinfo->stats.egr_drop_stats_count =
        fill_egr_drop_stats_(&dinfo->stats.egr_drop_stats[0]);
    return sdk::SDK_RET_OK;
}

sdk_ret_t
device_impl::program_mytep_(device_entry *device) {
    sdk_ret_t            ret;
    sdk_table_api_params_t params;
    tep_actiondata_t     tep_data = { 0 };
    nexthop_actiondata_t nh_data  = { 0 };
    uint32_t             tep_hw_id = PDS_IMPL_MYTEP_HW_ID;
    uint32_t             nh_hw_id  = PDS_IMPL_MYTEP_NEXTHOP_HW_ID;
    ip_addr_t            tep_ip_addr = device->ip_addr();

    if (getenv("APOLLO_TEST_TEP_ENCAP")) {
        tep_data.action_id = TEP_IPV4_VXLAN_TEP_ID;
        tep_data.tep_ipv4_vxlan_action.dipo = tep_ip_addr.addr.v4_addr;
        memcpy(tep_data.tep_ipv4_vxlan_action.dmac, device->mac(), ETH_ADDR_LEN);
    } else {
        tep_data.action_id = TEP_MPLS_UDP_TEP_ID;
        tep_data.tep_mpls_udp_action.dipo = tep_ip_addr.addr.v4_addr;
        memcpy(tep_data.tep_mpls_udp_action.dmac, device->mac(), ETH_ADDR_LEN);
    }

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&params, tep_hw_id, &tep_data, NULL);
    ret = tep_impl_db()->tep_tbl()->insert_atid(&params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("TEP Tx table programming failed for TEP %s, "
                      "TEP hw id %u, err %u", ipaddr2str(&tep_ip_addr),
                      tep_hw_id, ret);
        return ret;
    }

    // program nexthop table
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.action_u.nexthop_nexthop_info.tep_index = tep_hw_id;

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&params, nh_hw_id, &nh_data, NULL);
    ret = nexthop_impl_db()->nh_tbl()->insert_atid(&params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Nexthop Tx table programming failed for TEP %s, "
                      "nexthop hw id %u, err %u", ipaddr2str(&tep_ip_addr),
                      nh_hw_id, ret);
    }
    PDS_TRACE_DEBUG("Programmed TEP %s, MAC %s, hw id %u, nexthop id %u",
                    ipaddr2str(&tep_ip_addr),
                    macaddr2str(device->mac()), tep_hw_id, nh_hw_id);
    return ret;
}

sdk_ret_t
device_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                         pds_epoch_t epoch, api_op_t api_op,
                         api_obj_ctxt_t *obj_ctxt) {
    device_entry *device;
    ip_addr_t ip_addr;
    sdk_ret_t ret = SDK_RET_OK;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        device = (device_entry *)api_obj;
        ip_addr = device->ip_addr();
        PDS_TRACE_DEBUG("Activating device IP %s, MAC %s as table constants",
                        ipaddr2str(&ip_addr), macaddr2str(device->mac()));
        if (ip_addr.af == IP_AF_IPV4) {
            sdk::asic::pd::asicpd_program_table_constant(
                               P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                               ip_addr.addr.v4_addr);
            sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP,
                                                         ip_addr.addr.v4_addr);
        }
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP,
                                                     MAC_TO_UINT64(device->mac()));
        ret = program_mytep_(device);
        break;

    case API_OP_DELETE:
        PDS_TRACE_DEBUG("Cleaning up device config");
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, 0);
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP, 0);
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP, 0);
        break;

    default:
        return SDK_RET_INVALID_OP;
    }
    return ret;
}

/// \@}

}    // namespace impl
}    // namespace api
