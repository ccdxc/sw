//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of vnic
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/impl/artemis/route_impl.hpp"
#include "nic/apollo/api/impl/artemis/security_policy_impl.hpp"
#include "nic/apollo/api/impl/artemis/meter_impl.hpp"
#include "nic/apollo/api/impl/artemis/vnic_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL - vnic entry datapath implementation
/// \ingroup PDS_VNIC
/// \@{

vnic_impl *
vnic_impl::factory(pds_vnic_spec_t *spec) {
    vnic_impl *impl;

    // TODO: move to slab later
    impl = (vnic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VNIC_IMPL,
                                   sizeof(vnic_impl));
    new (impl) vnic_impl();
    return impl;
}

void
vnic_impl::destroy(vnic_impl *impl) {
    impl->~vnic_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_VNIC_IMPL, impl);
}

sdk_ret_t
vnic_impl::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    sdk_table_api_params_t api_params = { 0 };
    vnic_mapping_swkey vnic_mapping_key = { 0 };
    vnic_mapping_swkey_mask_t vnic_mapping_mask = { 0 };
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;

    // allocate hw id for this vnic
    if (vnic_impl_db()->vnic_idxr()->alloc(&idx) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate hw id for vnic %u", spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    hw_id_ = idx;

    // NOTE: we don't need to reserve indices in INGRESS_VNIC_INFO and
    //       EGRESS_VNIC_INFO, as we use hw_id_ as index into those tables

    // reserve an entry in VNIC_MAPPING table
    if (spec->switch_vnic) {
        vnic_mapping_key.ctag_1_vid = spec->fabric_encap.val.vlan_tag;
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
    } else {
        if ((spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) &&
            spec->vnic_encap.val.vlan_tag) {
            vnic_mapping_key.ctag_1_vid = spec->vnic_encap.val.vlan_tag;
        }
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
        sdk::lib::memrev(vnic_mapping_key.ethernet_1_srcAddr, spec->mac_addr,
                         ETH_ADDR_LEN);
        memset(vnic_mapping_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    }
    api_params.key = &vnic_mapping_key;
    api_params.mask = &vnic_mapping_mask;
    api_params.handle = sdk::table::handle_t::null();
    ret = vnic_impl_db()->vnic_mapping_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in VNIC_MAPPING "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }
    vnic_mapping_handle_ = api_params.handle;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (hw_id_ != 0xFFFF) {
        api_params.handle = vnic_mapping_handle_;
        vnic_impl_db()->vnic_mapping_tbl()->release(&api_params);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (hw_id_ != 0xFFFF) {
        api_params.handle = vnic_mapping_handle_;
        vnic_impl_db()->vnic_mapping_tbl()->remove(&api_params);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

#define rxdma_vnic_info    action_u.vnic_info_rxdma_vnic_info_rxdma
#define txdma_vnic_info    action_u.vnic_info_txdma_vnic_info_txdma
sdk_ret_t
vnic_impl::program_vnic_info_(vpc_entry *vpc, subnet_entry *subnet,
                              pds_vnic_spec_t *spec)
{
    sdk_ret_t ret;
    mem_addr_t addr;
    policy *sec_policy;
    meter_entry *meter;
    route_table *rtable;
    p4pd_error_t p4pd_ret;
    pds_policy_key_t policy_key;
    pds_route_table_key_t route_table_key;
    vnic_info_rxdma_actiondata_t tx_rxdma_vnic_info_data = { 0 };
    vnic_info_rxdma_actiondata_t rx_rxdma_vnic_info_data = { 0 };
    vnic_info_txdma_actiondata_t txdma_vnic_info_data = { 0 };

    // prepare VNIC_INFO_RXDMA entries in RX & TX directions
    tx_rxdma_vnic_info_data.action_id = VNIC_INFO_RXDMA_VNIC_INFO_RXDMA_ID;
    rx_rxdma_vnic_info_data.action_id = VNIC_INFO_RXDMA_VNIC_INFO_RXDMA_ID;

    // prepare VNIC_INFO_TXDMA entry
    txdma_vnic_info_data.action_id = VNIC_INFO_TXDMA_VNIC_INFO_TXDMA_ID;

    // egress v4 & v6 policy roots are programmed in TX direction entry
    policy_key = subnet->egr_v4_policy(0);
    sec_policy = policy_db()->find(&policy_key);
    if (sec_policy) {
        addr = ((impl::security_policy_impl *)(sec_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv4 egr policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base1,
                                addr, 5);
    }
    policy_key = subnet->egr_v6_policy(0);
    sec_policy = policy_db()->find(&policy_key);
    if (sec_policy) {
        addr = ((impl::security_policy_impl *)(sec_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv6 egr policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base2,
                                addr, 5);
    }

    // ingress v4 & v6 policy roots are programmed in RX direction entry
    policy_key = subnet->ing_v4_policy(0);
    sec_policy = policy_db()->find(&policy_key);
    if (sec_policy) {
        addr = ((impl::security_policy_impl *)(sec_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv4 ing policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base1,
                                addr, 5);
    }
    policy_key = subnet->ing_v6_policy(0);
    sec_policy = policy_db()->find(&policy_key);
    if (sec_policy) {
        addr = ((impl::security_policy_impl *)(sec_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv6 ing policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base2,
                                addr, 5);
    }

    meter = meter_db()->find(&spec->v4_meter);
    if (meter) {
        addr = ((impl::meter_impl *)(meter->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv4 meter root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base3,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base3,
                                addr, 5);
    }
    meter = meter_db()->find(&spec->v6_meter);
    if (meter) {
        addr = ((impl::meter_impl *)(meter->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 meter root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base4,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base4,
                                addr, 5);
    }

    route_table_key = subnet->v4_route_table();
    rtable = route_table_db()->find(&route_table_key);
    if (rtable) {
        addr =
            ((impl::route_table_impl *)(rtable->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv4 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base5,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base5,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(txdma_vnic_info_data.txdma_vnic_info.lpm_base1,
                                addr, 5);
    }

    route_table_key = subnet->v6_route_table();
    if (route_table_key.id != PDS_ROUTE_TABLE_ID_INVALID) {
        rtable = route_table_db()->find(&route_table_key);
    } else {
        rtable = NULL;
    }
    if (rtable) {
        addr = ((impl::route_table_impl *)(rtable->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(tx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base6,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(rx_rxdma_vnic_info_data.rxdma_vnic_info.lpm_base6,
                                addr, 5);
        MEM_ADDR_TO_P4_MEM_ADDR(txdma_vnic_info_data.txdma_vnic_info.lpm_base2,
                                addr, 5);
    }

    // program VNIC_INFO_RXDMA entry for TX direction in 2nd half of the table
    // at hw_id_ index
    p4pd_ret = p4pd_global_entry_write(P4_P4PLUS_RXDMA_TBL_ID_VNIC_INFO_RXDMA,
                                       VNIC_INFO_TABLE_SIZE + hw_id_,
                                       NULL, NULL, &tx_rxdma_vnic_info_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VNIC_INFO_RXDMA table at %u",
                      VNIC_INFO_TABLE_SIZE + hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // program VNIC_INFO_RXDMA entry for RX direction at hw_id_ index
    p4pd_ret = p4pd_global_entry_write(P4_P4PLUS_RXDMA_TBL_ID_VNIC_INFO_RXDMA,
                                       hw_id_, NULL, NULL,
                                       &rx_rxdma_vnic_info_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VNIC_INFO_RXDMA table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // program TXDMA_VNIC_INFO entry at hw_id_ index
    p4pd_ret = p4pd_global_entry_write(P4_P4PLUS_TXDMA_TBL_ID_VNIC_INFO_TXDMA,
                                       hw_id_, NULL, NULL,
                                       &txdma_vnic_info_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VNIC_INFO_TXDMA table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

#define ingress_vnic_info_action    action_u.ingress_vnic_info_ingress_vnic_info
#define egress_vnic_info_action     action_u.egress_vnic_info_egress_vnic_info
#define local_46_mapping_action     action_u.local_46_mapping_local_46_info
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    mem_addr_t addr;
    device_entry *device;
    subnet_entry *subnet;
    p4pd_error_t p4pd_ret;
    pds_vnic_spec_t *spec;
    pds_vpc_key_t vpc_key;
    pds_subnet_key_t subnet_key;
    vnic_rx_stats_actiondata_t vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata_t vnic_tx_stats_data = { 0 };
    egress_vnic_info_actiondata_t egr_vnic_info = { 0 };
    local_46_mapping_actiondata_t local_46_mapping_data = { 0 };

    device = device_db()->find();
    spec = &obj_ctxt->api_params->vnic_spec;

    // get the subnet of the vnic
    subnet = subnet_db()->find(&spec->subnet);
    if (unlikely(subnet == NULL)) {
        PDS_TRACE_ERR("Unable to find subnet : %u, vpc : %u",
                      spec->subnet.id, spec->vpc.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

    // get the vpc of this subnet
    vpc_key = subnet->vpc();
    vpc = vpc_db()->find(&vpc_key);
    if (unlikely(vpc == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    // program LOCAL_46_MAPPING table
    if (vpc->nat46_prefix_valid()) {
        local_46_mapping_data.action_id = LOCAL_46_MAPPING_LOCAL_46_INFO_ID;
        sdk::lib::memrev(local_46_mapping_data.local_46_mapping_action.prefix,
                         vpc->nat46_prefix().addr.addr.v6_addr.addr8,
                         IP6_ADDR8_LEN);
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_LOCAL_46_MAPPING,
                                           hw_id_ + 1, NULL, NULL,
                                           &local_46_mapping_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program LOCAL_46_MAPPING table at %u",
                          hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
    }

    // initialize rx stats tables for this vnic
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VNIC_RX_STATS table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // program egress_vnic_info_table entry
    egr_vnic_info.action_id = EGRESS_VNIC_INFO_EGRESS_VNIC_INFO_ID;
    sdk::lib::memrev(egr_vnic_info.egress_vnic_info_action.vr_mac,
                     subnet->vr_mac(), ETH_ADDR_LEN);
    sdk::lib::memrev(egr_vnic_info.egress_vnic_info_action.ca_mac,
                     spec->mac_addr, ETH_ADDR_LEN);
    egr_vnic_info.egress_vnic_info_action.port = TM_PORT_UPLINK_0;
    egr_vnic_info.egress_vnic_info_action.local_vlan =
        spec->vnic_encap.val.value;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_EGRESS_VNIC_INFO,
                                       hw_id_, NULL, NULL,
                                       &egr_vnic_info);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program EGRESS_VNIC_INFO table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // initialize tx stats tables for this vnic
    vnic_tx_stats_data.action_id = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VNIC_TX_STATS table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // program vnic info tables in rxdma and txdma
    ret = program_vnic_info_(vpc, subnet, spec);
    return ret;
}

// TODO: when epoch support is added to these tables, we should pick
//       old epoch contents and override them !!!
sdk_ret_t
vnic_impl::reprogram_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    subnet_entry *subnet;
    pds_subnet_key_t subnet_key;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    egress_vnic_info_actiondata_t egress_vnic_data;

    subnet_key = vnic->subnet();
    subnet = (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, &subnet_key);

    // read EGRESS_VNIC_INFO table entry of this vnic
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_EGRESS_VNIC_INFO, hw_id_,
                                      NULL, NULL, &egress_vnic_data);
    if (p4pd_ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read EGRESS_VNIC_INFO table for "
                      "vnic %u at index %u", vnic->key().id, hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // update fields dependent on other objects and reprogram
    sdk::lib::memrev(egress_vnic_data.egress_vnic_info_action.vr_mac,
                     subnet->vr_mac(), ETH_ADDR_LEN);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_EGRESS_VNIC_INFO,
                                       hw_id_, NULL, NULL,
                                       &egress_vnic_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to update EGRESS_VNIC_INFO table at %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    // TODO:
    // 1. leave P4TBL_ID_VNIC_TX_STATS table entry as-is
    // 2. leave P4TBL_ID_VNIC_RX_STATS table entry as-is
    // 3. INGRESS_VNIC_INFO_TBL & EGRESS_VNIC_INFO_TBL index tables
    //    don't need to be invalidated, the table pointing to this will
    //    have latest epoch set with valid bit set to FALSE in activate_hw()
    //    stage
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define mapping_info_action    action_u.vnic_mapping_vnic_mapping_info
sdk_ret_t
vnic_impl::activate_vnic_create_(pds_epoch_t epoch, vnic_entry *vnic,
                                 pds_vnic_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;
    pds_vpc_key_t vpc_key;
    pds_subnet_key_t subnet_key;
    sdk_table_api_params_t api_params = { 0 };
    vnic_mapping_swkey vnic_mapping_key = { 0 };
    vnic_mapping_swkey_mask_t vnic_mapping_mask = { 0 };
    vnic_mapping_actiondata_t vnic_mapping_data = { 0 };

    subnet_key = vnic->subnet();
    subnet = subnet_db()->find(&subnet_key);
    if (unlikely(subnet == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    vpc_key = subnet->vpc();
    vpc = vpc_db()->find(&vpc_key);
    if (unlikely(vpc == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if (spec->switch_vnic) {
        vnic_mapping_key.ctag_1_vid = spec->fabric_encap.val.vlan_tag;
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
    } else {
        if ((spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) &&
            spec->vnic_encap.val.vlan_tag) {
            vnic_mapping_key.ctag_1_vid = spec->vnic_encap.val.vlan_tag;
        }
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
        sdk::lib::memrev(vnic_mapping_key.ethernet_1_srcAddr, spec->mac_addr,
                         ETH_ADDR_LEN);
        memset(vnic_mapping_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    }
    vnic_mapping_data.action_id = VNIC_MAPPING_VNIC_MAPPING_INFO_ID;
    vnic_mapping_data.mapping_info_action.epoch = epoch;
    vnic_mapping_data.mapping_info_action.vnic_id = hw_id_;
    if (vpc->type() == PDS_VPC_TYPE_UNDERLAY) {
        vnic_mapping_data.mapping_info_action.vpc_id =
            PDS_IMPL_PUBLIC_VPC_HW_ID;
    } else {
        vnic_mapping_data.mapping_info_action.vpc_id = vpc->hw_id();
    }
    api_params.key = &vnic_mapping_key;
    api_params.mask = &vnic_mapping_mask;
    api_params.appdata = &vnic_mapping_data;
    api_params.action_id = VNIC_MAPPING_VNIC_MAPPING_INFO_ID;
    api_params.handle = vnic_mapping_handle_;
    ret = vnic_impl_db()->vnic_mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNIC_MAPPING table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_delete_(pds_epoch_t epoch, vnic_entry *vnic) {
    // invalidate entry in tcam (but not release resources) ??
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, api_base *orig_obj, pds_epoch_t epoch,
                       api_op_t api_op, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t          ret;
    pds_vnic_spec_t    *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->vnic_spec;
        ret = activate_vnic_create_(epoch, (vnic_entry *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_vnic_delete_(epoch, (vnic_entry *)api_obj);
        break;

    case API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
vnic_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    sdk_table_api_params_t api_params = { 0 };
    vnic_mapping_swkey vnic_mapping_key = { 0 };
    vnic_mapping_swkey_mask_t vnic_mapping_mask = { 0 };
    vnic_mapping_actiondata_t vnic_mapping_data = { 0 };

    // read VNIC_MAPPING table entry
    api_params.key = &vnic_mapping_key;
    api_params.mask = &vnic_mapping_mask;
    api_params.appdata = &vnic_mapping_data;
    api_params.action_id = VNIC_MAPPING_VNIC_MAPPING_INFO_ID;
    api_params.handle = vnic_mapping_handle_;
    ret = vnic_impl_db()->vnic_mapping_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        //PDS_TRACE_ERR("Failed to read VNIC_MAPPING table for "
        //              "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }

    // update all the fields that depend on other objects
    // currently epoch is the only field that can change like this
    vnic_mapping_data.mapping_info_action.epoch = epoch;
    ret = vnic_impl_db()->vnic_mapping_tbl()->update(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNIC_MAPPING table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
        return ret;
    }

    return SDK_RET_OK;
}


#define vnic_tx_stats_action          action_u.vnic_tx_stats_vnic_tx_stats
#define vnic_rx_stats_action          action_u.vnic_rx_stats_vnic_rx_stats
sdk_ret_t
vnic_impl::fill_stats_(pds_vnic_stats_t *stats) {
    p4pd_error_t p4pd_ret;
    vnic_tx_stats_actiondata_t tx_stats = { 0 };
    vnic_rx_stats_actiondata_t rx_stats = { 0 };

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_TX_STATS, hw_id_, NULL,
                                      NULL, &tx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_TX_STATS table; hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_RX_STATS, hw_id_, NULL,
                                      NULL, &rx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_RX_STATS table hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    stats->tx_pkts  = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_packets;
    stats->tx_bytes = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_bytes;
    stats->rx_pkts  = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_packets;
    stats->rx_bytes = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_bytes;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::fill_spec_(pds_vnic_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    egress_vnic_info_actiondata_t egr_vnic_info = { 0 };

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_EGRESS_VNIC_INFO, hw_id_, NULL,
                                      NULL, &egr_vnic_info);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read EGRESS_VNIC_INFO table; hw_id:%u",
                      hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    sdk::lib::memrev(
        spec->mac_addr,
        egr_vnic_info.egress_vnic_info_action.ca_mac, ETH_ADDR_LEN);

    spec->vnic_encap.type = PDS_ENCAP_TYPE_DOT1Q;
    spec->vnic_encap.val.value =
        egr_vnic_info.egress_vnic_info_action.local_vlan;

    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_vnic_info_t *vnic_info = (pds_vnic_info_t *)info;

    rv = fill_spec_(&vnic_info->spec);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware spec tables for VNIC %s",
                      api_obj->key2str().c_str());
        return rv;
    }

    rv = fill_stats_(&vnic_info->stats);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware stats tables for VNIC %s",
                      api_obj->key2str().c_str());
        return rv;
    }

    return SDK_RET_OK;
}

/// \@}    // end of PDS_VNIC_IMPL

}    // namespace impl
}    // namespace api
