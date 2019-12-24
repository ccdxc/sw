//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP datapath implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/tep_impl.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - tep datapath implementation
/// \ingroup PDS_TEP
/// @{

#define tep_mpls_udp_action       action_u.tep_mpls_udp_tep
#define tep_ipv4_vxlan_action     action_u.tep_ipv4_vxlan_tep
#define tep_ipv6_vxlan_action     action_u.tep_ipv6_vxlan_tep
#define nh_action                 action_u.nexthop_nexthop_info

using sdk::table::sdk_table_api_params_t;

tep_impl *
tep_impl::factory(pds_tep_spec_t *pds_tep) {
    tep_impl *impl;

    // TODO: move to slab later
    impl = (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TEP_IMPL,
                                  sizeof(tep_impl));
    new (impl) tep_impl();
    return impl;
}

void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_TEP_IMPL, impl);
}

sdk_ret_t
tep_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t idx;
    sdk_table_api_params_t tparams;

    // reserve an entry in TEP_table
    api_obj->set_rsvd_rsc();
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, idx, NULL, NULL);
    ret = tep_impl_db()->tep_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in TEP table, err %u", ret);
        return ret;
    }
    idx = tparams.handle.pindex();
    hw_id_ = idx & 0xFFFF;

    // reserve an entry in NH_TX table
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, idx, NULL, NULL);
    ret = nexthop_impl_db()->nh_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in NH table, err %u", ret);
        return ret;
    }
    idx = tparams.handle.pindex();
    nh_id_ = idx & 0xFFFF;
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams;

    if (hw_id_ != 0xFFFF) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, hw_id_, NULL, NULL);
        tep_impl_db()->tep_tbl()->release(&tparams);
        hw_id_ = 0xFFFF;
    }
    if (nh_id_ != 0xFFFF) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, nh_id_, NULL, NULL);
        nexthop_impl_db()->nh_tbl()->release(&tparams);
        nh_id_ = 0xFFFF;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams;

    if (hw_id_ != 0xFFFF) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, hw_id_, NULL, NULL);
        tep_impl_db()->tep_tbl()->remove(&tparams);
        hw_id_ = 0xFFFF;
    }
    if (nh_id_ != 0xFFFF) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, nh_id_, NULL, NULL);
        nexthop_impl_db()->nh_tbl()->remove(&tparams);
        nh_id_ = 0xFFFF;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t               ret;
    tep_entry               *tep;
    device_entry            *device;
    pds_tep_spec_t          *tep_spec;
    tep_actiondata_t        tep_data = { 0 };
    nexthop_actiondata_t    nh_data = { 0 };
    sdk_table_api_params_t  api_params;

    // program TEP Tx table
    tep_spec = &obj_ctxt->api_params->tep_spec;
    tep = (tep_entry *)api_obj;
    ip_addr_t& tep_ip_addr = tep_spec->remote_ip;
    switch (tep_spec->encap.type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        tep_data.action_id = TEP_MPLS_UDP_TEP_ID;
        tep_data.tep_mpls_udp_action.dipo = tep_ip_addr.addr.v4_addr;
        memcpy(tep_data.tep_mpls_udp_action.dmac, tep->mac(), ETH_ADDR_LEN);
        break;

    case PDS_ENCAP_TYPE_VXLAN:
        if (tep_ip_addr.af == IP_AF_IPV4) {
            tep_data.action_id = TEP_IPV4_VXLAN_TEP_ID;
            tep_data.tep_ipv4_vxlan_action.dipo =
                tep_ip_addr.addr.v4_addr;
            memcpy(tep_data.tep_ipv4_vxlan_action.dmac,
                   tep->mac(), ETH_ADDR_LEN);
        } else {
            device = device_db()->find();
            tep_data.action_id = TEP_IPV6_VXLAN_TEP_ID;
            sdk::lib::memrev(tep_data.tep_ipv6_vxlan_action.sipo,
                             device->ip_addr().addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
            sdk::lib::memrev(tep_data.tep_ipv6_vxlan_action.dipo,
                             tep_ip_addr.addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
            memcpy(tep_data.tep_ipv6_vxlan_action.dmac,
                   tep->mac(), ETH_ADDR_LEN);
        }
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, hw_id_, &tep_data, NULL);
    ret = tep_impl_db()->tep_tbl()->insert_atid(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("TEP Tx table programming failed for TEP %s, "
                      "TEP hw id %u, err %u", api_obj->key2str().c_str(),
                      hw_id_, ret);
        return ret;
    }

    // program nexthop table
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nh_data.action_u.nexthop_nexthop_info.tep_index = hw_id_;
    if (tep_spec->nat) {
        nh_data.nh_action.snat_required = 1;
    }
    switch (tep_spec->type) {
    case PDS_TEP_TYPE_IGW:
        nh_data.nh_action.encap_type = GW_ENCAP;
        break;
    case PDS_TEP_TYPE_WORKLOAD:
        nh_data.nh_action.encap_type = VNIC_ENCAP;
        break;
    default:
        break;
    }
    if (tep_spec->encap.type != PDS_ENCAP_TYPE_NONE) {
        nh_data.action_u.nexthop_nexthop_info.dst_slot_id =
            tep_spec->encap.val.value;
    }

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params, nh_id_, &nh_data, NULL);
    ret = nexthop_impl_db()->nh_tbl()->insert_atid(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Nexthop Tx table programming failed for TEP %s, "
                      "nexthop hw id %u, err %u", api_obj->key2str().c_str(),
                      nh_id_, ret);
    }
    PDS_TRACE_DEBUG("Programmed TEP %s, MAC 0x%lx, hw id %u, nexthop id %u",
                    ipaddr2str(&tep_ip_addr),
                    PDS_REMOTE_TEP_MAC, hw_id_, nh_id_);
    return ret;
}

sdk_ret_t
tep_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    //TODO: need to update these entries or entries that are pointing to these
    // entries with new epoch and valid bit set to FALSE
    //if (hw_id_ != 0xFFFF) {
    //      tep_impl_db()->tep_tbl()->remove(hw_id_);
    //}
    //if (nh_id_ != 0xFFFF) {
    //    nexthop_impl_db()->nh_tbl()->remove(nh_id_);
    //}
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

void
tep_impl::fill_status_(tep_actiondata_t *tep_data,
                       pds_tep_status_t *status)
{
    status->nh_id = nh_id_;
    status->hw_id = hw_id_;

    switch (tep_data->action_id) {
    case TEP_GRE_TEP_ID:
        sdk::lib::memrev(status->dmac,
                         tep_data->action_u.tep_gre_tep.dmac,
                         ETH_ADDR_LEN);
        break;
    case TEP_MPLS_UDP_TEP_ID:
        sdk::lib::memrev(status->dmac,
                         tep_data->action_u.tep_mpls_udp_tep.dmac,
                         ETH_ADDR_LEN);
        break;
    case TEP_IPV4_VXLAN_TEP_ID:
        sdk::lib::memrev(status->dmac,
                         tep_data->action_u.tep_ipv4_vxlan_tep.dmac,
                         ETH_ADDR_LEN);
        break;
    }
}

void
tep_impl::fill_spec_(nexthop_actiondata_t *nh_data,
                     tep_actiondata_t *tep_data, pds_tep_spec_t *spec)
{
    if (tep_data->action_id == TEP_MPLS_UDP_TEP_ID) {
        spec->encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        spec->remote_ip.af = IP_AF_IPV4;
        spec->remote_ip.addr.v4_addr = tep_data->tep_mpls_udp_action.dipo;
        memcpy(spec->mac, tep_data->action_u.tep_mpls_udp_tep.dmac,
               ETH_ADDR_LEN);
        switch (nh_data->nh_action.encap_type) {
        case GW_ENCAP:
            spec->type = PDS_TEP_TYPE_IGW;
            break;
        case VNIC_ENCAP:
            spec->type = PDS_TEP_TYPE_WORKLOAD;
            break;
        }
        spec->encap.val.mpls_tag =
            nh_data->action_u.nexthop_nexthop_info.dst_slot_id;
    } else if (tep_data->action_id == TEP_IPV4_VXLAN_TEP_ID) {
        spec->encap.type = PDS_ENCAP_TYPE_VXLAN;
        spec->encap.val.vnid =
            nh_data->action_u.nexthop_nexthop_info.dst_slot_id;
        spec->remote_ip.af = IP_AF_IPV4;
        spec->remote_ip.addr.v4_addr = tep_data->tep_ipv4_vxlan_action.dipo;
        memcpy(spec->mac, tep_data->action_u.tep_ipv4_vxlan_tep.dmac,
               ETH_ADDR_LEN);
    }
    spec->nat = nh_data->action_u.nexthop_nexthop_info.snat_required;
}

sdk_ret_t
tep_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    nexthop_actiondata_t nh_data;
    tep_actiondata_t tep_data;
    sdk_table_api_params_t tparams;
    pds_tep_info_t *tep_info = (pds_tep_info_t *)info;
    (void)api_obj;
    (void)key;

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, nh_id_, &nh_data, NULL);
    if (nexthop_impl_db()->nh_tbl()->get(&tparams) != SDK_RET_OK) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&tparams, hw_id_, &tep_data, NULL);
    if (tep_impl_db()->tep_tbl()->get(&tparams) != SDK_RET_OK) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    fill_spec_(&nh_data, &tep_data, &tep_info->spec);
    fill_status_(&tep_data, &tep_info->status);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
