/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl.cc
 *
 * @brief   datapath implementation of vnic
 */

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

/**
 * @defgroup PDS_VNIC_IMPL - vnic entry datapath implementation
 * @ingroup PDS_VNIC
 * @{
 */

/**
 * @brief    factory method to allocate & initialize vnic impl instance
 * @param[in] spec    vnic information
 * @return    new instance of vnic or NULL, in case of error
 */
vnic_impl *
vnic_impl::factory(pds_vnic_spec_t *spec) {
    vnic_impl *impl;

    // TODO: move to slab later
    impl = (vnic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VNIC_IMPL,
                                  sizeof(vnic_impl));
    new (impl) vnic_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given vnic,
 *           if any, and free the memory
 * @param[in] vnic     vnic to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
vnic_impl::destroy(vnic_impl *impl) {
    impl->~vnic_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_VNIC_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::reserve_resources(api_base *api_obj) {
    /**
     * allocate hw id for this vnic, vnic specific index tables in the p4
     * datapath are indexed by this
     */
    if (vnic_impl_db()->vnic_idxr()->alloc((uint32_t *)&hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
#define egress_local_vnic_info_rx_action    action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_impl::release_resources(api_base *api_obj) {
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };

    if (hw_id_ != 0xFF) {
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
        // TODO: how do we know whether we actually programmed h/w or not ?
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_,
                                                                  &egress_vnic_data);
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->remove(hw_id_);
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->remove(
            egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id);
        if (vnic_by_slot_hash_idx_ != 0xFFFF) {
            vnic_impl_db()->local_vnic_by_slot_rx_tbl()->remove(vnic_by_slot_hash_idx_);
        }
    }
    return SDK_RET_OK;
}

#define vnic_tx_stats_action action_u.vnic_tx_stats_vnic_tx_stats
#define vnic_rx_stats_action action_u.vnic_rx_stats_vnic_rx_stats
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

sdk_ret_t
vnic_impl::read_hw(pds_vnic_key_t *key, pds_vnic_info_t *info) {
    p4pd_error_t p4pd_ret;
    vnic_tx_stats_actiondata_t vnic_tx_stats_data;
    vnic_rx_stats_actiondata_t vnic_rx_stats_data;

    // read the current entry in h/w
    p4pd_ret = p4pd_entry_read(P4TBL_ID_VNIC_TX_STATS, hw_id_, NULL, NULL,
                               &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    p4pd_ret = p4pd_entry_read(P4TBL_ID_VNIC_RX_STATS, hw_id_, NULL, NULL,
                               &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    fill_vnic_stats_(&vnic_tx_stats_data, &vnic_rx_stats_data, &info->stats);
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
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

    spec= &obj_ctxt->api_params->vnic_info;
    subnet = subnet_db()->subnet_find(&spec->subnet);
    if (subnet == NULL) {
        PDS_TRACE_ERR("Unable to find subnet : %u, vcn : %u",
                      spec->subnet.id, spec->vcn.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< initialize tx stats tables for this vnic */
    vnic_tx_stats_data.action_id = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    /**< initialize egress_local_spec_rx table entry */
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
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->insert_withid(&egress_vnic_data,
                                                                         hw_id_);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< initialize rx stats tables for this vnic */
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] curr_obj    cloned and updated version of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define local_vnic_by_vlan_tx_info    action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info    action_u.local_vnic_by_slot_rx_local_vnic_info_rx
#define MEM_ADDR_TO_P4_MEM_ADDR(p4_mem_addr, mem_addr, p4_addr_size)      \
    for (uint32_t i = 0; i < (p4_addr_size); i++) {                       \
        p4_mem_addr[i] = ((mem_addr) >> (i * 8)) & 0xFF;                  \
    }

/**
 * @brief    program LOCAL_VNIC_BY_VLAN_TX table and activate the epoch in
 *           the Tx direction
 * @param[in] api_op         api operation
 * @param[in] api_obj        vnic entry object
 * @param[in] epoch          epoch being activated
 * @param[in] vcn            vcn entry
 * @param[in] subnet         subnet entry
 * @param[in] spec           vnic configuration
 * @param[in] v4_route_table IPv4 routing table entry
 * @param[in] v6_route_table IPv6 routing table entry
 * @param[in] v4_policy      egress IPv4 security policy
 * @param[in] v6_policy      egress IPv6 security policy
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::activate_vnic_by_vlan_tx_table_(api_op_t api_op, api_base *api_obj,
                                           pds_epoch_t epoch, vcn_entry *vcn,
                                           subnet_entry *subnet,
                                           pds_vnic_spec_t *spec,
                                           route_table *v4_route_table,
                                           route_table *v6_route_table,
                                           policy *v4_policy,
                                           policy *v6_policy) {
    sdk_ret_t                             ret;
    mem_addr_t                            addr;
    local_vnic_by_vlan_tx_actiondata_t    vnic_by_vlan_data = { 0 };

    switch (api_op) {
    case api::API_OP_CREATE:
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
        ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->insert_withid(&vnic_by_vlan_data,
                                                                         spec->wire_vlan);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }

    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_VLAN_TX table failed, "
                      "api op %u, epoch %u, vnic %s , err %u", api_op, epoch,
                      api_obj->key2str().c_str(), ret);
    }
    return ret;
}

/**
 * @brief    program LOCAL_VNIC_BY_SLOT_RX table and activate the epoch in
 *           the Rx direction
 * @param[in] api_op         api operation
 * @param[in] api_obj        vnic entry object
 * @param[in] epoch          epoch being activated
 * @param[in] vcn            vcn entry
 * @param[in] subnet         subnet entry
 * @param[in] spec           vnic configuration
 * @param[in] v4_policy      ingress IPv4 security policy
 * @param[in] v6_policy      ingress IPv6 security policy
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::activate_vnic_by_slot_rx_table_(api_op_t api_op, api_base *api_obj,
                                           pds_epoch_t epoch, vcn_entry *vcn,
                                           subnet_entry *subnet,
                                           pds_vnic_spec_t *spec,
                                           policy *v4_policy,
                                           policy *v6_policy) {
    sdk_ret_t                             ret;
    mem_addr_t                            addr;
    local_vnic_by_slot_rx_swkey_t         vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t    vnic_by_slot_data = { 0 };

    switch (api_op) {
    case api::API_OP_CREATE:
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
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of LOCAL_VNIC_BY_SLOT_RX table failed, "
                      "api op %u, epoch %u, vnic %s , err %u", api_op, epoch,
                       api_obj->key2str().c_str(), ret);
        }
    return ret;
}

/**
 * @brief    activate the epoch in the dataplane by programming stage 0
 *           tables, if any
 * @param[in] epoch       epoch being activated
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                ret;
    vcn_entry                *vcn;
    subnet_entry             *subnet;
    pds_vnic_spec_t          *spec;
    pds_route_table_key_t    route_table_key;
    route_table              *v4_route_table, *v6_route_table;
    pds_policy_key_t         policy_key;
    policy                   *ing_v4_policy, *ing_v6_policy;
    policy                   *egr_v4_policy, *egr_v6_policy;

    spec = &obj_ctxt->api_params->vnic_info;
    vcn = vcn_db()->vcn_find(&spec->vcn);
    if (vcn == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    subnet = subnet_db()->subnet_find(&spec->subnet);
    if (subnet == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    route_table_key = subnet->v4_route_table();
    v4_route_table = route_table_db()->route_table_find(&route_table_key);
    route_table_key = subnet->v4_route_table();
    if (route_table_key.id != PDS_ROUTE_TABLE_ID_INVALID) {
        v6_route_table =
            route_table_db()->route_table_find(&route_table_key);
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

    switch (api_op) {
    case api::API_OP_CREATE:
        /**< program local_vnic_by_vlan_tx table entry */
        ret = activate_vnic_by_vlan_tx_table_(api_op, api_obj, epoch, vcn,
                                              subnet, spec, v4_route_table,
                                              v6_route_table, egr_v4_policy,
                                              egr_v6_policy);
        if (ret == SDK_RET_OK) {
            /**< program local_vnic_by_slot_rx table entry */
            ret = activate_vnic_by_slot_rx_table_(api_op, api_obj, epoch, vcn,
                                                  subnet, spec,
                                                  ing_v4_policy, ing_v6_policy);
        }
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

/** @} */    // end of PDS_VNIC_IMPL

}    // namespace impl
}    // namespace api
