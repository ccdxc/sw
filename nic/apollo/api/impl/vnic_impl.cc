//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of vnic
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/impl/route_impl.hpp"
#include "nic/apollo/api/impl/security_policy_impl.hpp"
#include "nic/apollo/api/impl/vnic_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

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
vnic_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t          ret;
    uint32_t           idx;
    pds_vnic_spec_t    *spec = &obj_ctxt->api_params->vnic_spec;

    // allocate hw id for this vnic
    if (vnic_impl_db()->vnic_idxr()->alloc(&idx) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate hw id for vnic %u", spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    hw_id_ = idx;

    // reserve an entry in LOCAL_VNIC_BY_VLAN_TX table
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->reserve_index(spec->wire_vlan);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_VNIC_BY_VLAN_TX "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }

    // reserve an entry in EGRESS_LOCAL_VNIC_INFO_RX table
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->reserve_index(hw_id_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in EGRESS_LOCAL_VNIC_INFO_RX "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }

    // TODO: sdk_hash table doesn't have reserve()/release() APIs, reserve an
    //       index when that is available
    return SDK_RET_OK;
}

#define egress_local_vnic_info_rx_action    action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_impl::release_resources(api_base *api_obj) {
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };

    if (hw_id_ != 0xFFFF) {
        // TODO: uncomment the below once sdk_hash moves to standard API model
        //if (vnic_by_slot_hash_idx_ != SDK_TABLE_HANDLE_INVALID) {
            //vnic_impl_db()->local_vnic_by_slot_rx_tbl()->release(vnic_by_slot_hash_idx_);
        //}
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_,
                                                                  &egress_vnic_data);
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->release(
            egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id);
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->release(hw_id_);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);

    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::nuke_resources(api_base *api_obj) {
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };

    if (hw_id_ != 0xFFFF) {
        // TODO: uncomment the below once sdk_hash moves to standard API model
        //if (vnic_by_slot_hash_idx_ != SDK_TABLE_HANDLE_INVALID) {
        if (vnic_by_slot_hash_idx_ != 0xFFFF) {
            vnic_impl_db()->local_vnic_by_slot_rx_tbl()->remove(vnic_by_slot_hash_idx_);
        }
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_,
                                                                  &egress_vnic_data);
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->remove(
            egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id);
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->remove(hw_id_);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

// TODO: undo stuff if something goes wrong here !!
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                                 ret;
    pds_vnic_spec_t                           *spec;
    p4pd_error_t                              p4pd_ret;
    subnet_entry                              *subnet;
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata_t                vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata_t                vnic_tx_stats_data = { 0 };

    spec = &obj_ctxt->api_params->vnic_spec;
    subnet = subnet_db()->find(&spec->subnet);
    if (subnet == NULL) {
        PDS_TRACE_ERR("Unable to find subnet : %u, vcn : %u",
                      spec->subnet.id, spec->vcn.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

    // initialize tx stats tables for this vnic
    vnic_tx_stats_data.action_id = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // initialize egress_local_vnic_info_rx table entry
    egress_vnic_data.action_id =
        EGRESS_LOCAL_VNIC_INFO_RX_EGRESS_LOCAL_VNIC_INFO_RX_ID;
    sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_rx_action.vr_mac,
                     subnet->vr_mac(), ETH_ADDR_LEN);
    sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_rx_action.overlay_mac,
                     spec->mac_addr, ETH_ADDR_LEN);
    egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id =
        spec->wire_vlan;
    egress_vnic_data.egress_local_vnic_info_rx_action.subnet_id =
        subnet->hw_id();
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->insert_atid(&egress_vnic_data,
                                                                       hw_id_);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // TODO: remove once retrieve() issue is fixed
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_, &egress_vnic_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("READING BACK FAILED AT %u", hw_id_);
    } else {
        PDS_TRACE_ERR("READING BACK SUCCESS AT %u", hw_id_);
        PDS_TRACE_DEBUG("vlan id %u, subnet %u",
                        egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
                        egress_vnic_data.egress_local_vnic_info_rx_action.subnet_id);
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

sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    // TODO:
    // 1. leave P4TBL_ID_VNIC_TX_STATS table entry as-is
    // 2. leave VNIC_RX_STATS_VNIC_RX_STATS_ID table entry as-is
    // 3. EGRESS_LOCAL_VNIC_INFO_RX_TBL in index table and doesn't
    //    need to be invalidated, the table pointing to this will
    //    have latest epoch set with valid bit set to FALSE in
    //    activate_hw() stage
    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define vnic_tx_stats_action action_u.vnic_tx_stats_vnic_tx_stats
#define vnic_rx_stats_action action_u.vnic_rx_stats_vnic_rx_stats
#define local_vnic_by_vlan_tx_info    action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info    action_u.local_vnic_by_slot_rx_local_vnic_info_rx
#define MEM_ADDR_TO_P4_MEM_ADDR(p4_mem_addr, mem_addr, p4_addr_size)      \
    for (uint32_t i = 0; i < (p4_addr_size); i++) {                       \
        p4_mem_addr[i] = ((mem_addr) >> (i * 8)) & 0xFF;                  \
    }

sdk_ret_t
vnic_impl::activate_vnic_by_vlan_tx_table_create_(pds_epoch_t epoch,
                                                  vcn_entry *vcn,
                                                  pds_vnic_spec_t *spec,
                                                  vnic_entry *vnic,
                                                  route_table *v4_route_table,
                                                  route_table *v6_route_table,
                                                  policy *v4_policy,
                                                  policy *v6_policy) {
    sdk_ret_t                             ret;
    mem_addr_t                            addr;
    local_vnic_by_vlan_tx_actiondata_t    vnic_by_vlan_data;

    memset(&vnic_by_vlan_data, 0, sizeof(vnic_by_vlan_data));
    vnic_by_vlan_data.action_id =
        LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag = hw_id_;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vcn_id = vcn->hw_id();
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check =
        spec->src_dst_check ? false : true;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_1 =
        spec->rsc_pool_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_2 = 0;

    // program the LPM tree base address
    addr =
        ((impl::route_table_impl *)(v4_route_table->impl()))->lpm_root_addr();
    PDS_TRACE_DEBUG("IPv4 lpm root addr 0x%llx", addr);
    MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v4addr_1,
                            addr, 5);
    if (v6_route_table) {
        addr =
            ((impl::route_table_impl *)(v6_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_v6addr_1,
                                addr, 5);
    }

    // program security policy block's base address
    if (v4_policy) {
        addr = ((impl::security_policy_impl *)(v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v4addr_1,
                                addr, 5);
    }
    if (v6_policy) {
        addr = ((impl::security_policy_impl *)(v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Egress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.sacl_v6addr_1,
                                addr, 5);
    }
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = epoch;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2 = PDS_EPOCH_INVALID;
    sdk::lib::memrev(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.overlay_mac,
                     spec->mac_addr, ETH_ADDR_LEN);
    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.src_slot_id =
            spec->fabric_encap.val.mpls_tag;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.src_slot_id =
            spec->fabric_encap.val.vnid;
    }
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->insert_atid(&vnic_by_vlan_data,
                                                                   spec->wire_vlan);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_VLAN_TX table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_by_slot_rx_table_create_(pds_epoch_t epoch,
                                                  vcn_entry *vcn,
                                                  pds_vnic_spec_t *spec,
                                                  vnic_entry *vnic,
                                                  policy *v4_policy,
                                                  policy *v6_policy) {
    sdk_ret_t ret;
    mem_addr_t                            addr;
    local_vnic_by_slot_rx_swkey_t         vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t    vnic_by_slot_data = { 0 };

    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = spec->fabric_encap.val.mpls_tag;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    }
    vnic_by_slot_data.action_id =
        LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag = hw_id_;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.vcn_id = vcn->hw_id();
    vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check =
        spec->src_dst_check ? false : true;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_1 =
        spec->rsc_pool_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_2 = 0;

    // program security policy block's base address
    if (v4_policy) {
        addr = ((impl::security_policy_impl *)(v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v4addr_1,
                                addr, 5);
    } else {
        // TODO: hack - pls don't commit
        addr = security_policy_impl_db()->security_policy_region_addr();
        PDS_TRACE_DEBUG("Ingress IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v4addr_1,
                                addr, 5);
    }
    if (v6_policy) {
        addr = ((impl::security_policy_impl *)(v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("Ingress IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(vnic_by_slot_data.local_vnic_by_slot_rx_info.sacl_v6addr_1,
                                addr, 5);
    }
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = epoch;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2 = PDS_EPOCH_INVALID;
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->insert(&vnic_by_slot_key,
                                                              &vnic_by_slot_data,
                                                              (uint32_t *)&vnic_by_slot_hash_idx_);
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
    pds_vcn_key_t            vcn_key;
    vcn_entry                *vcn;
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
    vcn_key = subnet->vcn();
    vcn = vcn_db()->find(&vcn_key);
    if (unlikely(vcn == NULL)) {
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

    policy_key = subnet->ing_v4_policy();
    ing_v4_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->ing_v6_policy();
    ing_v6_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->egr_v4_policy();
    egr_v4_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->egr_v6_policy();
    egr_v6_policy = policy_db()->policy_find(&policy_key);
    // program local_vnic_by_vlan_tx table entry
    ret = activate_vnic_by_vlan_tx_table_create_(epoch, vcn, spec, vnic,
                                                 v4_route_table, v6_route_table,
                                                 egr_v4_policy, egr_v6_policy);
    if (ret == SDK_RET_OK) {
        // program local_vnic_by_slot_rx table entry
        ret = activate_vnic_by_slot_rx_table_create_(epoch, vcn, spec, vnic,
                                                     ing_v4_policy,
                                                     ing_v6_policy);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_delete_(pds_epoch_t epoch, vnic_entry *vnic) {
    sdk_ret_t                                 ret;
    local_vnic_by_vlan_tx_actiondata_t        vnic_by_vlan_data;
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data;
    local_vnic_by_slot_rx_swkey_t             vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t        vnic_by_slot_data;

    // read EGRESS_LOCAL_VNIC_INFO_RX table entry first
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(
              hw_id_, &egress_vnic_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read EGRESS_LOCAL_VNIC_INFO_RX entry "
                      "at %u, err %u", hw_id_, ret);
        return ret;
    }

    // then read LOCAL_VNIC_BY_VLAN_TX table entry for this vnic using the
    // wire vlan
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->retrieve(
              egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
              &vnic_by_vlan_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read LOCAL_VNIC_BY_VLAN_TX entry "
                      "at %u, err %u",
                      egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
                      ret);
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
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->update(
              egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
              &vnic_by_vlan_data);

    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed at deactivate LOCAL_VNIC_BY_VLAN_TX entry "
                      "at %u, err %u",
                      egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
                      ret);
        // fall thru and attempt further cleanup !!
    }

    // read LOCAL_VNIC_BY_SLOT_RX table entry for this vnic using the slot id
    if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = vnic->fabric_encap().val.mpls_tag;
    } else if (vnic->fabric_encap().type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = vnic->fabric_encap().val.vnid;;
    }
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->retrieve(vnic_by_slot_hash_idx_,
                                                                &vnic_by_slot_key,
                                                                &vnic_by_slot_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to find entry in LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, hw id %u, key = (mpls tag %u, vni %u), err %u",
                      egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
                      hw_id_, vnic_by_slot_key.mpls_dst_label,
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
    ret =  vnic_impl_db()->local_vnic_by_slot_rx_tbl()->update(vnic_by_slot_hash_idx_,
                                                               &vnic_by_slot_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate LOCAL_VNIC_BY_SLOT_RX table for "
                      "vnic %u, hw id %u, key = (mpls tag %u, vni %u), err %u",
                      egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
                      hw_id_, vnic_by_slot_key.mpls_dst_label,
                      vnic_by_slot_key.vxlan_1_vni, ret);
    }
    return ret;
}

sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t          ret;
    pds_vnic_spec_t    *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->vnic_spec;
        ret = activate_vnic_create_(epoch, (vnic_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_vnic_delete_(epoch, (vnic_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
vnic_impl::fill_vnic_stats_(vnic_tx_stats_actiondata_t *tx_stats,
                            vnic_rx_stats_actiondata_t *rx_stats,
                            pds_vnic_stats_t *stats)
{
    stats->tx_pkts  = *(uint64_t *)tx_stats->vnic_tx_stats_action.out_packets;
    stats->tx_bytes = *(uint64_t *)tx_stats->vnic_tx_stats_action.out_bytes;
    stats->rx_pkts  = *(uint64_t *)rx_stats->vnic_rx_stats_action.in_packets;
    stats->rx_bytes = *(uint64_t *)rx_stats->vnic_rx_stats_action.in_bytes;
    return;
}

void
vnic_impl::fill_vnic_spec_(
                egress_local_vnic_info_rx_actiondata_t *egress_vnic_data,
                local_vnic_by_vlan_tx_actiondata_t     *vnic_by_vlan_data,
                local_vnic_by_slot_rx_swkey_t          *vnic_by_slot_key,
                local_vnic_by_slot_rx_actiondata_t     *vnic_by_slot_data,
                pds_vnic_spec_t *spec) {
    // from EGRESS_LOCAL_VNIC_INFO_RX table
    sdk::lib::memrev(
        spec->mac_addr,
        egress_vnic_data->egress_local_vnic_info_rx_action.overlay_mac,
        ETH_ADDR_LEN);
    spec->wire_vlan =
        egress_vnic_data->egress_local_vnic_info_rx_action.overlay_vlan_id;
    spec->subnet.id =
        egress_vnic_data->egress_local_vnic_info_rx_action.subnet_id;
    // from VNIC_BY_VLAN_TX table
    spec->vcn.id = vnic_by_vlan_data->local_vnic_by_vlan_tx_info.vcn_id;
    spec->src_dst_check =
        vnic_by_vlan_data->local_vnic_by_vlan_tx_info.skip_src_dst_check == true
            ? false : true;
    spec->rsc_pool_id =
        vnic_by_vlan_data->local_vnic_by_vlan_tx_info.resource_group_1;
    // from VNIC_BY_SLOT_RX table
    if (vnic_by_slot_key->vxlan_1_vni != 0) {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        spec->fabric_encap.val.vnid = vnic_by_slot_key->vxlan_1_vni;
    } else {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        spec->fabric_encap.val.mpls_tag = vnic_by_slot_key->mpls_dst_label;
    }
}

sdk_ret_t
vnic_impl::read_hw(pds_vnic_key_t *key, pds_vnic_info_t *info) {
    p4pd_error_t p4pd_ret;
    sdk_ret_t ret;
    vnic_tx_stats_actiondata_t vnic_tx_stats_data;
    vnic_rx_stats_actiondata_t vnic_rx_stats_data;
    egress_local_vnic_info_rx_actiondata_t egress_vnic_data = { 0 };
    local_vnic_by_vlan_tx_actiondata_t     vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_swkey_t          vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t     vnic_by_slot_data = { 0 };

    // read VNIC_TX_STATS and VNIC_RX_STATS
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_TX_STATS, hw_id_, NULL,
                                      NULL, &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_RX_STATS, hw_id_, NULL,
                                      NULL, &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    // read EGRESS_LOCAL_VNIC_INFO_RX table
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(
              hw_id_, &egress_vnic_data);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // read LOCAL_VNIC_BY_VLAN_TX table
    ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->retrieve(
              egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id,
              &vnic_by_vlan_data);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // read LOCAL_VNIC_BY_SLOT_RX table, this is to get encap type
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->retrieve(
                                            vnic_by_slot_hash_idx_,
                                            &vnic_by_slot_key,
                                            &vnic_by_slot_data);
     if (ret != SDK_RET_OK) {
        return ret;
    }

    // fill stats
    fill_vnic_stats_(&vnic_tx_stats_data, &vnic_rx_stats_data, &info->stats);
    fill_vnic_spec_(&egress_vnic_data, &vnic_by_vlan_data,
                    &vnic_by_slot_key, &vnic_by_slot_data, &info->spec);

    PDS_TRACE_DEBUG("wire vlan %u, overlay mac %s, subnet %u, vcn %u",
                    info->spec.wire_vlan, macaddr2str(info->spec.mac_addr),
                    info->spec.subnet.id, info->spec.vcn.id);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_VNIC_IMPL

}    // namespace impl
}    // namespace api
