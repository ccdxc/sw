//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of VNIC
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/route_impl.hpp"
#include "nic/apollo/api/impl/apollo/security_policy_impl.hpp"
#include "nic/apollo/api/impl/apollo/vnic_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL - VNIC entry datapath implementation
/// \ingroup PDS_VNIC
/// @{

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
    sdk_table_api_params_t tparams = { 0 };
    sdk_table_api_params_t api_params;
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;
    local_vnic_by_vlan_tx_swkey_t local_vnic_by_vlan_tx_key = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t local_vnic_by_vlan_tx_mask = { 0 };
    local_vnic_by_slot_rx_swkey_t local_vnic_by_slot_rx_key = { 0 };

    // allocate hw id for this vnic
    if (vnic_impl_db()->vnic_idxr()->alloc(&idx) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate hw id for vnic %u", spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    hw_id_ = idx;

    // reserve an entry in LOCAL_VNIC_BY_VLAN_TX table
    if (!spec->switch_vnic) {
        if (spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            local_vnic_by_vlan_tx_key.ctag_1_vid =
                spec->vnic_encap.val.vlan_tag;
        }
        local_vnic_by_vlan_tx_mask.ctag_1_vid_mask = ~0;
        sdk::lib::memrev(local_vnic_by_vlan_tx_key.ethernet_1_srcAddr,
                         spec->mac_addr, ETH_ADDR_LEN);
        memset(local_vnic_by_vlan_tx_mask.ethernet_1_srcAddr_mask, 0xFF,
               ETH_ADDR_LEN);
    }
    tparams.key = &local_vnic_by_vlan_tx_key;
    tparams.mask = &local_vnic_by_vlan_tx_mask;
    tparams.handle = sdk::table::handle_t::null();
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_VNIC_BY_VLAN_TX "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }
    local_vnic_by_vlan_tx_handle_ = tparams.handle;

    // reserve an entry in EGRESS_LOCAL_VNIC_INFO table
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, hw_id_, NULL, NULL);
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->reserve_index(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in EGRESS_LOCAL_VNIC_INFO "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }

    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        local_vnic_by_slot_rx_key.mpls_dst_label =
            spec->fabric_encap.val.mpls_tag;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        local_vnic_by_slot_rx_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_vnic_by_slot_rx_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_VNIC_BY_SLOT_RX "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }
    local_vnic_by_slot_rx_handle_ = api_params.handle;
    return SDK_RET_OK;
}

#define egress_local_vnic_info_action    action_u.egress_local_vnic_info_egress_local_vnic_info
sdk_ret_t
vnic_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (hw_id_ != 0xFFFF) {
        api_params.handle = local_vnic_by_slot_rx_handle_;
        vnic_impl_db()->local_vnic_by_slot_rx_tbl()->release(&api_params);

        api_params.handle = local_vnic_by_vlan_tx_handle_;
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->release(&api_params);
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_, NULL, NULL);
        vnic_impl_db()->egress_local_vnic_info_tbl()->release(&api_params);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);

    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };
    sdk_table_api_params_t slot_api_params = { 0 };
    local_vnic_by_vlan_tx_swkey_t vnic_by_vlan_key = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t vnic_by_vlan_mask = { 0 };
    local_vnic_by_slot_rx_swkey_t local_vnic_by_slot_rx_key = { 0 };
    vnic_entry *vnic = (vnic_entry *)api_obj;
    pds_encap_t vnic_encap = vnic->vnic_encap();

    if (hw_id_ != 0xFFFF) {
        if (local_vnic_by_slot_rx_handle_ != SDK_TABLE_HANDLE_INVALID) {
            if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_MPLSoUDP) {
                local_vnic_by_slot_rx_key.mpls_dst_label = vnic->fabric_encap().val.mpls_tag;
            } else if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_VXLAN) {
                local_vnic_by_slot_rx_key.vxlan_1_vni = vnic->fabric_encap().val.vnid;
            }
            PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &local_vnic_by_slot_rx_key,
                                           NULL, NULL, 0, local_vnic_by_slot_rx_handle_);
            vnic_impl_db()->local_vnic_by_slot_rx_tbl()->remove(&slot_api_params);
        }
        if (vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            vnic_by_vlan_key.ctag_1_vid = vnic_encap.val.vlan_tag;
        }
        vnic_by_vlan_mask.ctag_1_vid_mask = ~0;
        sdk::lib::memrev(vnic_by_vlan_key.ethernet_1_srcAddr, vnic->mac(),
                         ETH_ADDR_LEN);
        memset(vnic_by_vlan_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
        api_params.key = &vnic_by_vlan_key;
        api_params.mask = &vnic_by_vlan_mask;
        api_params.handle = local_vnic_by_vlan_tx_handle_;
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->remove(&api_params);

        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_, NULL, NULL);
        vnic_impl_db()->egress_local_vnic_info_tbl()->remove(&api_params);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    this->release_resources(api_obj);
    return SDK_RET_OK;
}

// TODO: undo stuff if something goes wrong here !!
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                                 ret;
    pds_vnic_spec_t                           *spec;
    p4pd_error_t                              p4pd_ret;
    subnet_entry                              *subnet;
    egress_local_vnic_info_actiondata_t       egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata_t                vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata_t                vnic_tx_stats_data = { 0 };
    sdk_table_api_params_t                    api_params;

    spec = &obj_ctxt->api_params->vnic_spec;
    subnet = subnet_db()->find(&spec->subnet);
    if (subnet == NULL) {
        PDS_TRACE_ERR("Unable to find subnet %u, vpc %u",
                      spec->subnet.id, spec->vpc.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

    if (!spec->switch_vnic) {
        // initialize tx stats tables for this vnic
        vnic_tx_stats_data.action_id = VNIC_TX_STATS_VNIC_TX_STATS_ID;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                           hw_id_, NULL, NULL,
                                           &vnic_tx_stats_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        // initialize egress_local_vnic_info_table entry
        egress_vnic_data.action_id =
            EGRESS_LOCAL_VNIC_INFO_EGRESS_LOCAL_VNIC_INFO_ID;
        sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_action.vr_mac,
                         subnet->vr_mac(), ETH_ADDR_LEN);
        sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_action.overlay_mac,
                         spec->mac_addr, ETH_ADDR_LEN);

        if (spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            egress_vnic_data.egress_local_vnic_info_action.overlay_vlan_id =
                spec->vnic_encap.val.vlan_tag;
        }

        egress_vnic_data.egress_local_vnic_info_action.subnet_id =
            subnet->hw_id();
        if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
            egress_vnic_data.egress_local_vnic_info_action.src_slot_id =
                spec->fabric_encap.val.mpls_tag;
        } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
            egress_vnic_data.egress_local_vnic_info_action.src_slot_id =
                spec->fabric_encap.val.vnid;
        }
        if (spec->rx_mirror_session_bmap) {
            egress_vnic_data.egress_local_vnic_info_action.mirror_en = TRUE;
            egress_vnic_data.egress_local_vnic_info_action.mirror_session =
                spec->rx_mirror_session_bmap;
        }
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_,
                                              &egress_vnic_data, NULL);
        ret = vnic_impl_db()->egress_local_vnic_info_tbl()->insert_atid(&api_params);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    // initialize rx stats tables for this vnic
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

// TODO: when epoch support is added to these tables, we should pick
//       old epoch contents and override them !!!
sdk_ret_t
vnic_impl::reprogram_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_subnet_key_t subnet_key;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    subnet_entry *subnet;
    egress_local_vnic_info_actiondata_t egress_vnic_data = { 0 };
    sdk_table_api_params_t api_params;

    subnet_key = vnic->subnet();
    subnet = (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, &subnet_key);

    // read EGRESS_LOCAL_VNIC_INFO table entry of this vnic
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_,
                                          &egress_vnic_data, NULL);
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read EGRESS_LOCAL_VNIC_INFO table for "
                      "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }

    // update fields dependent on other objects and reprogram
    sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_action.vr_mac,
                     subnet->vr_mac(), ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_,
                                          &egress_vnic_data, NULL);
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->update(&api_params);
    return ret;
}

sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    // TODO:
    // 1. leave P4TBL_ID_VNIC_TX_STATS table entry as-is
    // 2. leave VNIC_RX_STATS_VNIC_RX_STATS_ID table entry as-is
    // 3. EGRESS_LOCAL_VNIC_INFO_TBL in index table and doesn't
    //    need to be invalidated, the table pointing to this will
    //    have latest epoch set with valid bit set to FALSE in
    //    activate_hw() stage
    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define vnic_tx_stats_action          action_u.vnic_tx_stats_vnic_tx_stats
#define vnic_rx_stats_action          action_u.vnic_rx_stats_vnic_rx_stats
#define local_vnic_by_vlan_tx_info    action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info    action_u.local_vnic_by_slot_rx_local_vnic_info_rx
sdk_ret_t
vnic_impl::activate_vnic_by_vlan_tx_table_create_(pds_epoch_t epoch,
                                                  vpc_entry *vpc,
                                                  pds_vnic_spec_t *spec,
                                                  vnic_entry *vnic,
                                                  route_table *v4_route_table,
                                                  route_table *v6_route_table,
                                                  policy *v4_policy,
                                                  policy *v6_policy) {
    sdk_ret_t                             ret;
    mem_addr_t                            addr;
    sdk_table_api_params_t                api_params = { 0 };
    local_vnic_by_vlan_tx_swkey_t         vnic_by_vlan_key = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t    vnic_by_vlan_mask = { 0 };
    local_vnic_by_vlan_tx_actiondata_t    vnic_by_vlan_data;

    memset(&vnic_by_vlan_data, 0, sizeof(vnic_by_vlan_data));
    vnic_by_vlan_data.action_id =
        LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag = hw_id_;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vpc_id = vpc->hw_id();
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check1 =
        spec->src_dst_check ? false : true;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check2 = false;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group2 = 0;

    // program the LPM tree base address
    if (v4_route_table) {
        addr =
            ((impl::route_table_impl *)(v4_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv4 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v4addr1,
                                addr, 5);
    }
    if (v6_route_table) {
        addr =
            ((impl::route_table_impl *)(v6_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v6addr1,
                                addr, 5);
    }

    // program security policy block's base address
    if (v4_policy) {
        addr = ((impl::security_policy_impl *)(v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v4addr1,
                                addr, 5);
    }
    if (v6_policy) {
        addr = ((impl::security_policy_impl *)(v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v6addr1,
                                addr, 5);
    }
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = epoch;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1_valid = true;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2 = PDS_EPOCH_INVALID;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2_valid = false;
    sdk::lib::memrev(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.overlay_mac1,
                     spec->mac_addr, ETH_ADDR_LEN);
    if (spec->tx_mirror_session_bmap) {
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.mirror_en = TRUE;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.mirror_session =
            spec->tx_mirror_session_bmap;
    }
    if (!spec->switch_vnic) {
        if (spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            vnic_by_vlan_key.ctag_1_vid = spec->vnic_encap.val.vlan_tag;
        }
        vnic_by_vlan_mask.ctag_1_vid_mask = ~0;
        sdk::lib::memrev(vnic_by_vlan_key.ethernet_1_srcAddr, spec->mac_addr,
                     ETH_ADDR_LEN);
        memset(vnic_by_vlan_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    }
    api_params.key = &vnic_by_vlan_key;
    api_params.mask = &vnic_by_vlan_mask;
    api_params.appdata = &vnic_by_vlan_data;
    api_params.action_id = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    api_params.handle = local_vnic_by_vlan_tx_handle_;
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_VLAN_TX table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_by_slot_rx_table_create_(pds_epoch_t epoch,
                                                  vpc_entry *vpc,
                                                  pds_vnic_spec_t *spec,
                                                  vnic_entry *vnic,
                                                  policy *v4_policy,
                                                  policy *v6_policy) {
    sdk_ret_t ret;
    mem_addr_t                            addr;
    local_vnic_by_slot_rx_swkey_t         vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t    vnic_by_slot_data = { 0 };
    sdk_table_api_params_t                api_params = { 0 };

    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = spec->fabric_encap.val.mpls_tag;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    }
    vnic_by_slot_data.action_id =
        LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag = hw_id_;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.vpc_id = vpc->hw_id();
    vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check1 =
        spec->src_dst_check ? false : true;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check2 = false;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group2 = 0;

    // program security policy block's base address
    if (v4_policy) {
        addr = ((impl::security_policy_impl *)(v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v4addr1,
                                addr, 5);
    }
    if (v6_policy) {
        addr = ((impl::security_policy_impl *)(v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v6addr1,
                                addr, 5);
    }
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = epoch;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1_valid = true;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2 = PDS_EPOCH_INVALID;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2_valid = false;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &vnic_by_slot_key,
                                   NULL, &vnic_by_slot_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   local_vnic_by_slot_rx_handle_);
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_SLOT_RX table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                       vnic->key2str().c_str(), ret);
    }

    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_create_(pds_epoch_t epoch, vnic_entry *vnic,
                                 pds_vnic_spec_t *spec) {
    sdk_ret_t                ret;
    pds_vpc_key_t            vpc_key;
    vpc_entry                *vpc;
    pds_subnet_key_t         subnet_key;
    subnet_entry             *subnet;
    pds_route_table_key_t    route_table_key;
    route_table              *v4_route_table, *v6_route_table;
    pds_policy_key_t         policy_key;
    policy                   *ing_v4_policy, *ing_v6_policy;
    policy                   *egr_v4_policy, *egr_v6_policy;

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
    route_table_key = subnet->v4_route_table();
    v4_route_table = route_table_db()->find(&route_table_key);
    route_table_key = subnet->v6_route_table();
    if (route_table_key.id != PDS_ROUTE_TABLE_ID_INVALID) {
        v6_route_table =
            route_table_db()->find(&route_table_key);
    } else {
        v6_route_table = NULL;
    }

    policy_key = subnet->ing_v4_policy(0);
    ing_v4_policy = policy_db()->find(&policy_key);
    policy_key = subnet->ing_v6_policy(0);
    ing_v6_policy = policy_db()->find(&policy_key);
    policy_key = subnet->egr_v4_policy(0);
    egr_v4_policy = policy_db()->find(&policy_key);
    policy_key = subnet->egr_v6_policy(0);
    egr_v6_policy = policy_db()->find(&policy_key);
    // program local_vnic_by_vlan_tx table entry
    ret = activate_vnic_by_vlan_tx_table_create_(epoch, vpc, spec, vnic,
                                                 v4_route_table, v6_route_table,
                                                 egr_v4_policy, egr_v6_policy);
    if (ret == SDK_RET_OK) {
        // program local_vnic_by_slot_rx table entry
        ret = activate_vnic_by_slot_rx_table_create_(epoch, vpc, spec, vnic,
                                                     ing_v4_policy,
                                                     ing_v6_policy);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_delete_(pds_epoch_t epoch, vnic_entry *vnic) {
    sdk_ret_t                                 ret;
    sdk_table_api_params_t                    api_params = { 0 };
    sdk_table_api_params_t                    slot_api_params = { 0 };
    local_vnic_by_vlan_tx_swkey_t             vnic_by_vlan_key = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t        vnic_by_vlan_mask = { 0 };
    local_vnic_by_vlan_tx_actiondata_t        vnic_by_vlan_data;
    egress_local_vnic_info_actiondata_t       egress_vnic_data;
    local_vnic_by_slot_rx_swkey_t             vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t        vnic_by_slot_data;
    pds_encap_t                               vnic_encap = vnic->vnic_encap();

    // read LOCAL_VNIC_BY_VLAN_TX table entry for this vnic
    if (vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
        vnic_by_vlan_key.ctag_1_vid = vnic_encap.val.vlan_tag;
    }
    vnic_by_vlan_mask.ctag_1_vid_mask = ~0;
    sdk::lib::memrev(vnic_by_vlan_key.ethernet_1_srcAddr, vnic->mac(),
                     ETH_ADDR_LEN);
    memset(vnic_by_vlan_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    api_params.key = &vnic_by_vlan_key;
    api_params.mask = &vnic_by_vlan_mask;
    api_params.appdata = &vnic_by_vlan_data;
    api_params.action_id = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    api_params.handle = local_vnic_by_vlan_tx_handle_;
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_VLAN_TX entry "
                      "for vnic %, err %u", vnic->key().id, ret);
        return ret;
    }

    if (vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 <
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2) {
        // update data corresponding to epoch1
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = epoch;
        //vnic_by_vlan_data.local_vnic_by_vlan_tx_info.valid1 = FALSE;
    } else {
        // update data corresponding to epoch2
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2 = epoch;
        //vnic_by_vlan_data.local_vnic_by_vlan_tx_info.valid2 = FALSE;
    }
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->update(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed at deactivate LOCAL_VNIC_BY_VLAN_TX entry for "
                      "vnic %u, err %u", vnic->key().id, ret);
        // fall thru and attempt further cleanup !!
    }

    // read LOCAL_VNIC_BY_SLOT_RX table entry for this vnic using the slot id
    if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = vnic->fabric_encap().val.mpls_tag;
    } else if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = vnic->fabric_encap().val.vnid;
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &vnic_by_slot_key,
                                   NULL, &vnic_by_slot_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   local_vnic_by_slot_rx_handle_);
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->get(&slot_api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to find entry in LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, hw id %u, key = (mpls tag %u, vni %u), err %u",
                      vnic->key().id, hw_id_,
                      vnic_by_slot_key.mpls_dst_label,
                      vnic_by_slot_key.vxlan_1_vni, ret);
        return ret;
    }

    if (vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 <
        vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2) {
        vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = epoch;
        //vnic_by_slot_data.local_vnic_by_slot_rx_info.valid1 = FALSE;
    } else {
        vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2 = epoch;
        //vnic_by_slot_data.local_vnic_by_slot_rx_info.valid2 = FALSE;
    }
    ret =  vnic_impl_db()->local_vnic_by_slot_rx_tbl()->update(&slot_api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, hw id %u, key = (mpls tag %u, "
                      "vni %u), err %u", vnic->key().id,
                      hw_id_, vnic_by_slot_key.mpls_dst_label,
                      vnic_by_slot_key.vxlan_1_vni, ret);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, api_base *orig_obj, pds_epoch_t epoch,
                       api_op_t api_op, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_vnic_spec_t *spec;

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

// TODO: when epoch support is added to these tables, we should pick
//       old epoch contents and override them !!!
sdk_ret_t
vnic_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    mem_addr_t addr;
    subnet_entry *subnet;
    pds_subnet_key_t subnet_key;
    pds_policy_key_t policy_key;
    pds_route_table_key_t route_table_key;
    policy *ing_v4_policy, *ing_v6_policy;
    policy *egr_v4_policy, *egr_v6_policy;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    sdk_table_api_params_t api_params = { 0 };
    sdk_table_api_params_t slot_api_params = { 0 };
    route_table *v4_route_table, *v6_route_table;
    local_vnic_by_vlan_tx_swkey_t vnic_by_vlan_key = { 0 };
    local_vnic_by_slot_rx_swkey_t vnic_by_slot_key = { 0 };
    egress_local_vnic_info_actiondata_t egress_vnic_data = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t vnic_by_vlan_mask = { 0 };
    local_vnic_by_vlan_tx_actiondata_t vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_actiondata_t vnic_by_slot_data = { 0 };
    pds_encap_t vnic_encap = vnic->vnic_encap();

    // read LOCAL_VNIC_BY_VLAN_TX table entry
    if (vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
        vnic_by_vlan_key.ctag_1_vid = vnic_encap.val.vlan_tag;
    }
    vnic_by_vlan_mask.ctag_1_vid_mask = ~0;
    sdk::lib::memrev(vnic_by_vlan_key.ethernet_1_srcAddr, vnic->mac(),
                     ETH_ADDR_LEN);
    memset(vnic_by_vlan_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    api_params.key = &vnic_by_vlan_key;
    api_params.mask = &vnic_by_vlan_mask;
    api_params.appdata = &vnic_by_vlan_data;
    api_params.action_id = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    api_params.handle = local_vnic_by_vlan_tx_handle_;
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_VLAN_TX table for "
                      "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }

    // and update all the data dependent on other objects
    subnet_key = vnic->subnet();
    subnet = (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, &subnet_key);
    route_table_key = subnet->v4_route_table();
    v4_route_table = (route_table *)api_base::find_obj(OBJ_ID_ROUTE_TABLE,
                                                       &route_table_key);
    route_table_key = subnet->v6_route_table();
    if (route_table_key.id != PDS_ROUTE_TABLE_ID_INVALID) {
        v6_route_table = (route_table *)api_base::find_obj(OBJ_ID_ROUTE_TABLE,
                                                           &route_table_key);
    } else {
        v6_route_table = NULL;
    }
    policy_key = subnet->ing_v4_policy(0);
    ing_v4_policy = (policy *)api_base::find_obj(OBJ_ID_POLICY, &policy_key);
    policy_key = subnet->ing_v6_policy(0);
    ing_v6_policy = (policy *)api_base::find_obj(OBJ_ID_POLICY, &policy_key);
    policy_key = subnet->egr_v4_policy(0);
    egr_v4_policy = (policy *)api_base::find_obj(OBJ_ID_POLICY, &policy_key);
    policy_key = subnet->egr_v6_policy(0);
    egr_v6_policy = (policy *)api_base::find_obj(OBJ_ID_POLICY, &policy_key);
    if (v4_route_table) {
        addr =
            ((impl::route_table_impl *)(v4_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv4 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v4addr1,
                                addr, 5);
    }
    if (v6_route_table) {
        addr =
            ((impl::route_table_impl *)(v6_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v6addr1,
                                addr, 5);
    }

    // program security policy block's base address
    if (egr_v4_policy) {
        addr = ((impl::security_policy_impl *)(egr_v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v4addr1,
                                addr, 5);
    }
    if (egr_v6_policy) {
        addr = ((impl::security_policy_impl *)(egr_v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v6addr1,
                                addr, 5);
    }
    // NOTE: ideally we should update portion of data that has min. epoch number
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = epoch;
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->update(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_VLAN_TX table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
        return ret;
    }

    // now, read LOCAL_VNIC_BY_SLOT_RX table, this is to get encap type
    if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = vnic->fabric_encap().val.mpls_tag;
    } else if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = vnic->fabric_encap().val.vnid;
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &vnic_by_slot_key,
                                   NULL, &vnic_by_slot_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   local_vnic_by_slot_rx_handle_);
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->get(&slot_api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }
    // program security policy block's base address
    if (ing_v4_policy) {
        addr = ((impl::security_policy_impl *)(ing_v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v4addr1,
                                addr, 5);
    }
    if (ing_v6_policy) {
        addr = ((impl::security_policy_impl *)(ing_v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v6addr1,
                                addr, 5);
    }
    // NOTE: ideally we should update portion of data that has min. epoch number
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = epoch;
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->update(&slot_api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_SLOT_RX table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                       vnic->key2str().c_str(), ret);
    }
    return ret;
}

void
vnic_impl::fill_status_(pds_vnic_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
vnic_impl::fill_stats_(pds_vnic_stats_t *stats) {
    p4pd_error_t p4pd_ret;
    vnic_tx_stats_actiondata_t tx_stats = { 0 };
    vnic_rx_stats_actiondata_t rx_stats = { 0 };

    // read P4TBL_ID_VNIC_TX_STATS table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_TX_STATS, hw_id_, NULL,
                                      NULL, &tx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_TX_STATS table; hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    stats->tx_pkts  = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_packets;
    stats->tx_bytes = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_bytes;

    // read P4TBL_ID_VNIC_RX_STATS table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_RX_STATS, hw_id_, NULL,
                                      NULL, &rx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_RX_STATS table hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    stats->rx_pkts  = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_packets;
    stats->rx_bytes = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_bytes;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::fill_spec_(pds_vnic_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    sdk_table_api_params_t api_params;
    sdk_table_api_params_t slot_api_params = { 0 };
    local_vnic_by_vlan_tx_swkey_t vnic_by_vlan_key = { 0 };
    local_vnic_by_vlan_tx_swkey_mask_t vnic_by_vlan_mask = { 0 };
    egress_local_vnic_info_actiondata_t egress_vnic_data = { 0 };
    local_vnic_by_vlan_tx_actiondata_t vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_swkey_t vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t vnic_by_slot_data = { 0 };
    pds_vnic_key_t *vkey = &spec->key;
    pds_vlan_id_t vnic_vlan;
    bool rx_src_dst_check, tx_src_dst_check;

    //TODO: epoch support
    // read EGRESS_LOCAL_VNIC_INFO table
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_,
                                          &egress_vnic_data, NULL);
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read EGRESS_LOCAL_VNIC_INFO table for "
                      "vnic %u, err %u", vkey->id, ret);
        return ret;
    }
    sdk::lib::memrev(
        spec->mac_addr,
        egress_vnic_data.egress_local_vnic_info_action.overlay_mac,
        ETH_ADDR_LEN);
    vnic_vlan = egress_vnic_data.egress_local_vnic_info_action.overlay_vlan_id;
    if (vnic_vlan) {
        spec->vnic_encap.type = PDS_ENCAP_TYPE_DOT1Q;
        spec->vnic_encap.val.vlan_tag = vnic_vlan;
    } else {
        spec->vnic_encap.type = PDS_ENCAP_TYPE_NONE;
    }
    spec->subnet.id =
        egress_vnic_data.egress_local_vnic_info_action.subnet_id;
    if (egress_vnic_data.egress_local_vnic_info_action.mirror_en == true) {
        spec->rx_mirror_session_bmap =
            egress_vnic_data.egress_local_vnic_info_action.mirror_session;
    }

    // read LOCAL_VNIC_BY_VLAN_TX table
    if (vnic_vlan) {
        vnic_by_vlan_key.ctag_1_vid = vnic_vlan;
    }
    vnic_by_vlan_mask.ctag_1_vid_mask = ~0;
    sdk::lib::memrev(
        vnic_by_vlan_key.ethernet_1_srcAddr,
        egress_vnic_data.egress_local_vnic_info_action.overlay_mac,
        ETH_ADDR_LEN);
    memset(vnic_by_vlan_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    api_params.key = &vnic_by_vlan_key;
    api_params.mask = &vnic_by_vlan_mask;
    api_params.appdata = &vnic_by_vlan_data;
    api_params.action_id = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    api_params.handle = local_vnic_by_vlan_tx_handle_;
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_VLAN_TX table for "
                      "vnic %u, err %u", vkey->id, ret);
        return ret;
    }
    spec->vpc.id = vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vpc_id;
    tx_src_dst_check =
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check1 == true
            ? false : true;
    if (vnic_by_vlan_data.local_vnic_by_vlan_tx_info.mirror_en == true) {
        spec->tx_mirror_session_bmap =
            vnic_by_vlan_data.local_vnic_by_vlan_tx_info.mirror_session;
    }

    // read LOCAL_VNIC_BY_SLOT_RX table, this is to get encap type
    PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &vnic_by_slot_key,
                                   NULL, &vnic_by_slot_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   local_vnic_by_slot_rx_handle_);
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->get(&slot_api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, err %u", vkey->id, ret);
        return ret;
    }
    rx_src_dst_check =
        vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check1 == true
            ? false : true;
    if (vnic_by_slot_key.vxlan_1_vni != 0) {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        spec->fabric_encap.val.vnid = vnic_by_slot_key.vxlan_1_vni;
    } else {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        spec->fabric_encap.val.mpls_tag = vnic_by_slot_key.mpls_dst_label;
    }
    SDK_ASSERT(rx_src_dst_check == tx_src_dst_check);
    spec->src_dst_check = rx_src_dst_check;

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
    fill_status_(&vnic_info->status);

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
