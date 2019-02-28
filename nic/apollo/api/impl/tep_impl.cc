/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep_impl.cc
 *
 * @brief   datapath implementation of tep
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/tep_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_TEP_IMPL - tep entry datapath implementation
 * @ingroup PDS_TEP
 * @{
 */

/**
 * @brief    factory method to allocate & initialize tep impl instance
 * @param[in] pds_tep    tep information
 * @return    new instance of tep or NULL, in case of error
 */
tep_impl *
tep_impl::factory(pds_tep_spec_t *pds_tep) {
    tep_impl *impl;

    // TODO: move to slab later
    impl = (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TEP_IMPL,
                                  sizeof(tep_impl));
    new (impl) tep_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given tep,
 *           if any, and free the memory
 * @param[in] tep     tep to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_TEP_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::reserve_resources(api_base *api_obj) {
    // TODO: if directmap provides a way to reserve() we dont need this indexer
    //       at all !!
    if (tep_impl_db()->tep_idxr()->alloc((uint32_t *)&hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    if (hw_id_ != 0xFF) {
        tep_impl_db()->tep_idxr()->free(hw_id_);
        tep_impl_db()->tep_tx_tbl()->remove(hw_id_);
    }
    return sdk::SDK_RET_INVALID_OP;
}

void
tep_impl::fill_status_(tep_tx_actiondata_t *tep_tx_data,
                       pds_tep_status_t *status)
{
    status->nh_id = nh_id_;
    status->hw_id = hw_id_;

    switch (tep_tx_data->action_id) {
    case TEP_TX_GRE_TEP_TX_ID:
        memcpy(status->dmac, tep_tx_data->action_u.tep_tx_gre_tep_tx.dmac,
               ETH_ADDR_LEN);
        break;
    case TEP_TX_MPLS_UDP_TEP_TX_ID:
        memcpy(status->dmac, tep_tx_data->action_u.tep_tx_mpls_udp_tep_tx.dmac,
               ETH_ADDR_LEN);
        break;
    }
}

void
tep_impl::fill_spec_(nexthop_tx_actiondata_t *nh_tx_data,
                     tep_tx_actiondata_t *tep_tx_data, pds_tep_spec_t *spec)
{
    switch (nh_tx_data->action_u.nexthop_tx_nexthop_info.encap_type) {
    case GW_ENCAP:
        spec->type = PDS_ENCAP_TYPE_GW_ENCAP;
        spec->key.ip_addr = tep_tx_data->action_u.tep_tx_mpls_udp_tep_tx.dipo;
        break;
    case VNIC_ENCAP:
        spec->type = PDS_ENCAP_TYPE_VNIC;
        spec->key.ip_addr = tep_tx_data->action_u.tep_tx_mpls_udp_tep_tx.dipo;
        break;
    }
}

sdk_ret_t
tep_impl::read_hw(pds_tep_info_t *info) {
    nexthop_tx_actiondata_t nh_tx_data;
    tep_tx_actiondata_t tep_tx_data;

    // TODO p4pd_entry_read does NOT seem to return proper data?
    if (p4pd_entry_read(P4TBL_ID_NEXTHOP_TX, nh_id_, NULL, NULL,
                        &nh_tx_data) != P4PD_SUCCESS) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if (p4pd_entry_read(P4TBL_ID_TEP_TX, hw_id_, NULL, NULL,
                        &tep_tx_data) != P4PD_SUCCESS) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    fill_spec_(&nh_tx_data, &tep_tx_data, &info->spec);
    fill_status_(&tep_tx_data, &info->status);

    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
#define tep_tx_mpls_udp_action    action_u.tep_tx_mpls_udp_tep_tx
#define tep_tx_vxlan_action       action_u.tep_tx_vxlan_tep_tx
#define nh_tx_action              action_u.nexthop_tx_nexthop_info
// TODO: fix this when fte plugin is available
#define PDS_REMOTE_TEP_MAC        0x0E0D0A0B0200
sdk_ret_t
tep_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                  ret;
    pds_tep_spec_t             *tep_spec;
    tep_tx_actiondata_t        tep_tx_data = { 0 };
    nexthop_tx_actiondata_t    nh_tx_data = { 0 };

    // program TEP Tx table
    tep_spec = &obj_ctxt->api_params->tep_spec;
    switch (tep_spec->type) {
    case PDS_ENCAP_TYPE_GW_ENCAP:
    case PDS_ENCAP_TYPE_VNIC:
        tep_tx_data.action_id = TEP_TX_MPLS_UDP_TEP_TX_ID;
        tep_tx_data.tep_tx_mpls_udp_action.dipo = tep_spec->key.ip_addr;
        MAC_UINT64_TO_ADDR(tep_tx_data.tep_tx_mpls_udp_action.dmac,
                           PDS_REMOTE_TEP_MAC);
        break;

    case PDS_ENCAP_TYPE_VXLAN:
        tep_tx_data.action_id = TEP_TX_VXLAN_TEP_TX_ID;
        tep_tx_data.tep_tx_vxlan_action.dipo = tep_spec->key.ip_addr;
        MAC_UINT64_TO_ADDR(tep_tx_data.tep_tx_vxlan_action.dmac,
                           PDS_REMOTE_TEP_MAC);
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    ret = tep_impl_db()->tep_tx_tbl()->insert_withid(&tep_tx_data, hw_id_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("TEP Tx table programming failed for TEP %s, "
                      "TEP hw id %u, err %u", api_obj->key2str().c_str(),
                      hw_id_, ret);
        return ret;
    }

    // program nexthop table
    nh_tx_data.action_id = NEXTHOP_TX_NEXTHOP_INFO_ID;
    nh_tx_data.action_u.nexthop_tx_nexthop_info.tep_index = hw_id_;
    switch (tep_spec->type) {
    case PDS_ENCAP_TYPE_GW_ENCAP:
        nh_tx_data.nh_tx_action.encap_type = GW_ENCAP;
        break;
    case PDS_ENCAP_TYPE_VNIC:
        nh_tx_data.nh_tx_action.encap_type = VNIC_ENCAP;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        nh_tx_data.nh_tx_action.encap_type = 0;    // don't care
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    ret = tep_impl_db()->nh_tx_tbl()->insert(&nh_tx_data, (uint32_t *)&nh_id_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Nexthop Tx table programming failed for TEP %s, "
                      "nexthop hw id %u, err %u", api_obj->key2str().c_str(),
                      hw_id_, ret);
    }
    PDS_TRACE_DEBUG("Programmed TEP %s, MAC 0x%lx, hw id %u, nexthop id %u",
                    ipv4addr2str(tep_spec->key.ip_addr),
                    PDS_REMOTE_TEP_MAC, hw_id_, nh_id_);

    return ret;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
tep_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    //TODO: need to unprogram HW.
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
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/** @} */    // end of PDS_TEP_IMPL

}    // namespace impl
}    // namespace api
