//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
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
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/impl/artemis/route_impl.hpp"
#include "nic/apollo/api/impl/artemis/security_policy_impl.hpp"
#include "nic/apollo/api/impl/artemis/vnic_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
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
    uint32_t idx;
    sdk_ret_t ret;
    sdk_table_api_params_t api_params = { 0 };
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;
    vnic_mapping_swkey vnic_mapping_key = { 0 };
    vnic_mapping_swkey_mask_t vnic_mapping_mask = { 0 };

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
    if ((spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) &&
        spec->vnic_encap.vlan) {
        vnic_mapping_key.ctag_1_vid = spec->vnic_encap.vla.vlan_tag;
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
    }
    sdk::lib::memrev(vnic_mapping_key.ethernet_1_srcAddr, spec->mac_addr,
                     ETH_ADDR_LEN);
    memset(vnic_mapping_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
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
        api_params.handle = local_vnic_by_vlan_tx_handle_;
        vnic_impl_db()->vnic_mapping_tbl()->remove(&api_params);
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

// TODO: undo stuff if something goes wrong here !!
#define ingress_vnic_info_action    action_u.ingress_vnic_info_ingress_vnic_info
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;
    p4pd_error_t p4pd_ret;
    pds_vnic_spec_t *spec;
    pds_vpc_key_t vpc_key;
    pds_subnet_key_t subnet_key;
    pds_policy_key_t policy_key;
    policy *ing_v4_policy, *ing_v6_policy;
    policy *egr_v4_policy, *egr_v6_policy;
    pds_route_table_key_t route_table_key;
    route_table *v4_route_table, *v6_route_table;
    //vnic_rx_stats_actiondata_t vnic_rx_stats_data = { 0 };
    //vnic_tx_stats_actiondata_t vnic_tx_stats_data = { 0 };
    ingress_vnic_info_actiondata_t ing_vnic_info = { 0 };
    //egress_vnic_info_actiondata_t egr_vnic_info = { 0 };

    spec = &obj_ctxt->api_params->vnic_spec;
    subnet = subnet_db()->find(&spec->subnet);
    if (unlikely(subnet == NULL)) {
        PDS_TRACE_ERR("Unable to find subnet : %u, vpc : %u",
                      spec->subnet.id, spec->vpc.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

#if 0
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
    // assert to support only dot1q encap for now
    SDK_ASSERT(spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q);
    egress_vnic_data.egress_local_vnic_info_action.overlay_vlan_id =
        spec->vnic_encap.val.vlan_tag;

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
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->insert_atid(&egress_vnic_data,
                                                                    hw_id_);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // initialize rx stats tables for this vnic
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
#endif

    // program INGRESS_VNIC_INFO table
#if 0
    vpc_key = subnet->vpc();
    vpc = vpc_db()->find(&vpc_key);
    if (unlikely(vpc == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }
#endif

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

    ing_vnic_info.action_id = INGRESS_VNIC_INFO_INGRESS_VNIC_INFO_ID;
    if (v4_route_table) {
        addr =
            ((impl::route_table_impl *)(v4_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv4 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(ing_vnic_info.ingress_vnic_info_action.v4_lpm,
                                addr, 5);
    }
    if (v6_route_table) {
        addr =
            ((impl::route_table_impl *)(v6_route_table->impl()))->lpm_root_addr();
        PDS_TRACE_DEBUG("IPv6 lpm root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(ing_vnic_info.ingress_vnic_info_action.v6_lpm,
                                addr, 5);
    }

    // TODO: we need to revisit once pipeline is fixed to take both ing & egr
    //       policy roots and potentially this table moves to RXDMA and/or TXDMA
    if (egr_v4_policy) {
        addr = ((impl::security_policy_impl *)(egr_v4_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv4 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(ing_vnic_info.ingress_vnic_info_action.v4_sacl,
                                addr, 5);
    }
    if (egr_v6_policy) {
        addr = ((impl::security_policy_impl *)(egr_v6_policy->impl()))->security_policy_root_addr();
        PDS_TRACE_DEBUG("IPv6 policy root addr 0x%llx", addr);
        MEM_ADDR_TO_P4_MEM_ADDR(ing_vnic_info.ingress_vnic_info_action.v6_sacl,
                                addr, 5);
    }
    // TODO: program v4 and v6 meter roots
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INGRESS_VNIC_INFO,
                                       hw_id_, NULL, NULL,
                                       &ing_vnic_info);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

// TODO: when epoch support is added to these tables, we should pick
//       old epoch contents and override them !!!
sdk_ret_t
vnic_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    // TODO: reprogram INGRESS_VNIC_INFO table and EGRESS_VNIC_INFO table
#if 0
    sdk_ret_t ret;
    pds_subnet_key_t subnet_key;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    subnet_entry *subnet;
    egress_local_vnic_info_actiondata_t egress_vnic_data = { 0 };

    subnet_key = vnic->subnet();
    subnet = (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, &subnet_key);

    // read EGRESS_LOCAL_VNIC_INFO table entry of this vnic
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->retrieve(
              hw_id_, &egress_vnic_data);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read EGRESS_LOCAL_VNIC_INFO table for "
                      "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }

    // update fields dependent on other objects and reprogram
    sdk::lib::memrev(egress_vnic_data.egress_local_vnic_info_action.vr_mac,
                     subnet->vr_mac(), ETH_ADDR_LEN);
    ret = vnic_impl_db()->egress_local_vnic_info_tbl()->update(hw_id_,
                                                               &egress_vnic_data);
#endif

    return ret;
}

sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    // TODO:
    // 1. leave P4TBL_ID_VNIC_TX_STATS table entry as-is
    // 2. leave P4TBL_ID_VNIC_RX_STATS table entry as-is
    // 3. INGRESS_VNIC_INFO_TBL & EGRESS_VNIC_INFO_TBL index tables
    //    don't need to be invalidated, the table pointing to this will
    //    have latest epoch set with valid bit set to FALSE in activate_hw()
    //    stage
    return sdk::SDK_RET_OK;
}

sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define MEM_ADDR_TO_P4_MEM_ADDR(p4_mem_addr, mem_addr, p4_addr_size)      \
    for (uint32_t i = 0; i < (p4_addr_size); i++) {                       \
        p4_mem_addr[i] = ((mem_addr) >> (i * 8)) & 0xFF;                  \
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

    if ((spec->vnic_encap.type == PDS_ENCAP_TYPE_DOT1Q) &&
        spec->vnic_encap.vlan) {
        vnic_mapping_key.ctag_1_vid = spec->vnic_encap.vla.vlan_tag;
        vnic_mapping_mask.ctag_1_vid_mask = ~0;
    }
    sdk::lib::memrev(vnic_mapping_key.ethernet_1_srcAddr, spec->mac_addr,
                     ETH_ADDR_LEN);
    memset(vnic_mapping_mask.ethernet_1_srcAddr_mask, 0xFF, ETH_ADDR_LEN);
    vnic_mapping_data.action_id = VNIC_MAPPING_VNIC_MAPPING_INFO_ID;
    vnic_mapping_data.mapping_info_action.epoch = epoch;
    vnic_mapping_data.mapping_info_action.vnic_id = hw_id_;
    vnic_mapping_data.mapping_info_action.vpc_id = vpc->hw_id();

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

sdk_ret_t
vnic_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_op_t api_op) {
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
        PDS_TRACE_ERR("Failed to read VNIC_MAPPING table for "
                      "vnic %u, err %u", vnic->key().id, ret);
        return ret;
    }

    // update all the fields that depend on other objects
    // currently epoch is the only field that can change like this
    vnic_mapping_data.epoch = epoch
    ret = vnic_impl_db()->vnic_mapping_tbl()->update(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNIC_MAPPING table failed, "
                      "epoch %u, vnic %s , err %u", epoch,
                      vnic->key2str().c_str(), ret);
        return ret;
    }

    return SDK_RET_OK;
}

void
vnic_impl::fill_vnic_stats_(vnic_tx_stats_actiondata_t *tx_stats,
                            vnic_rx_stats_actiondata_t *rx_stats,
                            pds_vnic_stats_t *stats)
{
    return;
}

void
vnic_impl::fill_vnic_spec_(
}

sdk_ret_t
vnic_impl::read_hw(obj_key_t *key, obj_info_t *info, void *arg) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_VNIC_IMPL

}    // namespace impl
}    // namespace api
