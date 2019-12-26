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
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/apollo/api/impl/apulu/device_impl.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_DEVICE_IMPL - device entry datapath implementation
/// \ingroup PDS_DEVICE
/// \@{

device_impl *
device_impl::factory(pds_device_spec_t *pds_device) {
    device_impl *impl;

    impl = (device_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_DEVICE_IMPL,
                                     sizeof(device_impl));
    new (impl) device_impl();
    return impl;
}

void
device_impl::destroy(device_impl *impl) {
    impl->~device_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_DEVICE_IMPL, impl);
}

impl_base *
device_impl::clone(void) {
    device_impl *cloned_impl;

    cloned_impl = (device_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_DEVICE_IMPL,
                                            sizeof(device_impl));
    new (cloned_impl) device_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
device_impl::free(device_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

#define p4i_device_info    action_u.p4i_device_info_p4i_device_info
#define p4e_device_info    action_u.p4e_device_info_p4e_device_info

sdk_ret_t
device_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                         pds_epoch_t epoch, api_op_t api_op,
                         api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    pds_device_spec_t *spec;
    p4i_device_info_actiondata_t p4i_device_info_data = { 0 };
    p4e_device_info_actiondata_t p4e_device_info_data = { 0 };

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->device_spec;
        PDS_TRACE_DEBUG("Activating device IP %s, MAC %s",
                        ipaddr2str(&spec->device_ip_addr),
                        macaddr2str(spec->device_mac_addr));

        // read the P4I_DEVICE_INFO table entry first as MACs are mostly
        // programmed by now during (inband) lif create
        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_P4I_DEVICE_INFO, 0,
                                           NULL, NULL, &p4i_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read P4I_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_P4E_DEVICE_INFO, 0,
                                           NULL, NULL, &p4e_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read P4E_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        // set up the data for ingress and egress device info table entries
        p4i_device_info_data.action_id = P4I_DEVICE_INFO_P4I_DEVICE_INFO_ID;
        p4e_device_info_data.action_id = P4E_DEVICE_INFO_P4E_DEVICE_INFO_ID;
        // populate/update the MyTEP IP
        if (spec->device_ip_addr.af == IP_AF_IPV4) {
            p4i_device_info_data.p4i_device_info.device_ipv4_addr =
                spec->device_ip_addr.addr.v4_addr;
            p4e_device_info_data.p4e_device_info.device_ipv4_addr =
                spec->device_ip_addr.addr.v4_addr;
        } else {
            sdk::lib::memrev(
                          p4i_device_info_data.p4i_device_info.device_ipv6_addr,
                          spec->device_ip_addr.addr.v6_addr.addr8,
                          IP6_ADDR8_LEN);
            sdk::lib::memrev(
                          p4e_device_info_data.p4e_device_info.device_ipv6_addr,
                          spec->device_ip_addr.addr.v6_addr.addr8,
                          IP6_ADDR8_LEN);
        }
        if (spec->bridging_en) {
            p4i_device_info_data.p4i_device_info.l2_enabled = TRUE;
        }
        // program the P4I_DEVICE_INFO table
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_P4I_DEVICE_INFO, 0,
                                           NULL, NULL, &p4i_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program P4I_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        // program the P4E_DEVICE_INFO table
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_P4E_DEVICE_INFO, 0,
                                           NULL, NULL, &p4e_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program P4E_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    case API_OP_DELETE:
        PDS_TRACE_DEBUG("Cleaning up device config");
        // program the P4I_DEVICE_INFO table
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_P4I_DEVICE_INFO, 0,
                                           NULL, NULL, &p4i_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program P4I_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        // program the P4E_DEVICE_INFO table
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_P4E_DEVICE_INFO, 0,
                                           NULL, NULL, &p4e_device_info_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program P4E_DEVICE_INFO table");
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    default:
        return SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
}

sdk_ret_t
device_impl::fill_spec_(pds_device_spec_t *spec) {
    p4pd_error_t p4pd_ret;
    p4i_device_info_actiondata_t device_info;

    // read P4I_DEVICE_INFO table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_P4I_DEVICE_INFO, 0,
                                      NULL, NULL, &device_info);
    if (unlikely(p4pd_ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read P4I_DEVICE_INFO table");
        return sdk::SDK_RET_HW_READ_ERR;
    }

    sdk::lib::memrev(spec->device_mac_addr,
                     device_info.p4i_device_info.device_mac_addr1,
                     ETH_ADDR_LEN);
    // check if there is an ipv4 address
    if (device_info.p4i_device_info.device_ipv4_addr) {
        spec->device_ip_addr.af = IP_AF_IPV4;
        spec->device_ip_addr.addr.v4_addr =
              device_info.p4i_device_info.device_ipv4_addr;
    } else {
        spec->device_ip_addr.af = IP_AF_IPV6;
        sdk::lib::memrev(spec->device_ip_addr.addr.v6_addr.addr8,
                         device_info.p4i_device_info.device_ipv6_addr,
                         IP6_ADDR8_LEN);
    }
    spec->bridging_en = device_info.p4i_device_info.l2_enabled;
    return SDK_RET_OK;
}

uint32_t
device_impl::fill_ing_drop_stats_(pds_device_drop_stats_t *ing_drop_stats) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_actiondata_t data;
    p4i_drop_stats_swkey_t key = { 0 };
    p4i_drop_stats_swkey_mask_t key_mask = { 0 };
    const char idrop[][PDS_MAX_DROP_NAME_LEN] = {
        "drop_vni_invalid",
        "drop_nacl",
        "drop_vnic_policer_tx",
        "drop_mac_ip_binding_fail",
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
        "drop_session_invalid",
        "drop_session_hit",
        "drop_nexthop_invalid",
        "drop_vnic_policer_rx",
        "drop_copp",
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
    sdk_ret_t          rv;

    (void)api_obj;
    (void)key;

    rv = fill_spec_(&dinfo->spec);
    if (unlikely(rv != SDK_RET_OK)) {
        return rv;
    }
    // TODO: rename ing_drop_stats_count and egr_drop_stats_count to
    //       num_ing_drop_stats and num_egr_drop_stats respectively
    dinfo->stats.ing_drop_stats_count =
        fill_ing_drop_stats_(&dinfo->stats.ing_drop_stats[0]);
    dinfo->stats.egr_drop_stats_count =
        fill_egr_drop_stats_(&dinfo->stats.egr_drop_stats[0]);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
