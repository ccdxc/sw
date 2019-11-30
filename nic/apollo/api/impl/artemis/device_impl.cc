//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of device
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/artemis/device_impl.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "gen/p4gen/artemis/include/p4pd.h"

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
    uint64_t val;

    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_VNIC_MAPPING,
                                              &val);
    if (val) {
        spec->device_ip_addr.af = IP_AF_IPV4;
        spec->device_ip_addr.addr.v4_addr = (ipv4_addr_t)val;
    }
    sdk::asic::pd::asicpd_read_table_constant(P4TBL_ID_TEP1_RX, &val);
    MAC_UINT64_TO_ADDR(spec->device_mac_addr, val);
}

void
device_impl::fill_ing_drop_stats_(pds_device_drop_stats_t *ing_drop_stats) {
#if 1
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = { 0 };
    p4i_drop_stats_swkey_mask_t key_mask = { 0 };
    p4i_drop_stats_actiondata_t data = { 0 };

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; ++i) {
        if (p4pd_global_entry_read(P4TBL_ID_P4I_DROP_STATS, i,
                                   &key, &key_mask, &data) == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts));
            ing_drop_stats[i].count = pkts;
            // TODO Fill name for the stats
        }
    }
#endif
    return;
}

sdk_ret_t
device_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    pds_device_info_t *dinfo = (pds_device_info_t *)info;
    (void)api_obj;
    (void)key;

    fill_spec_(&dinfo->spec);
    fill_ing_drop_stats_(&dinfo->stats.ing_drop_stats[0]);
    return sdk::SDK_RET_OK;
}

sdk_ret_t
device_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                         pds_epoch_t epoch, api_op_t api_op,
                         obj_ctxt_t *obj_ctxt) {
    device_entry *device;
    ip_addr_t ip_addr;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        device = (device_entry *)api_obj;
        ip_addr = device->ip_addr();
        PDS_TRACE_DEBUG("Activating device IP %s, MAC %s as table constants",
                        ipaddr2str(&ip_addr), macaddr2str(device->mac()));
        if (ip_addr.af == IP_AF_IPV4) {
            sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_VNIC_MAPPING,
                                                         ip_addr.addr.v4_addr);
            sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP,
                                                         ip_addr.addr.v4_addr);
        }
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_EGRESS_VNIC_INFO,
                                                     MAC_TO_UINT64(device->mac()));
        break;

    case API_OP_DELETE:
        PDS_TRACE_DEBUG("Cleaning up device config");
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_VNIC_MAPPING, 0);
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP, 0);
        sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_EGRESS_VNIC_INFO, 0);
        break;

    default:
        return SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
