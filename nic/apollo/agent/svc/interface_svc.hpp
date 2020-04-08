//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for interface object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_INTERFACE_SVC_HPP__
#define __AGENT_SVC_INTERFACE_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

static inline pds::IfStatus
pds_admin_state_to_proto_admin_state (pds_if_state_t state)
{
    switch (state) {
    case PDS_IF_STATE_DOWN:
        return pds::IF_STATUS_DOWN;
    case PDS_IF_STATE_UP:
        return pds::IF_STATUS_UP;
    default:
        return pds::IF_STATUS_NONE;
    }
}

static inline pds::IfType
pds_if_type_to_proto_if_type (pds_if_type_t type)
{
    switch (type) {
    case PDS_IF_TYPE_UPLINK:
        return pds::IF_TYPE_UPLINK;
    case PDS_IF_TYPE_UPLINK_PC:
        return pds::IF_TYPE_UPLINK_PC;
    case PDS_IF_TYPE_L3:
        return pds::IF_TYPE_L3;
    case PDS_IF_TYPE_LOOPBACK:
        return pds::IF_TYPE_LOOPBACK;
    case PDS_IF_TYPE_CONTROL:
        return pds::IF_TYPE_CONTROL;
    default:
        return pds::IF_TYPE_NONE;
    }
}

static inline void
pds_if_api_spec_to_proto (pds::InterfaceSpec *proto_spec,
                          const pds_if_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_type(pds_if_type_to_proto_if_type(api_spec->type));
    proto_spec->set_adminstatus(
                pds_admin_state_to_proto_admin_state(api_spec->admin_state));

    switch (api_spec->type) {
    case PDS_IF_TYPE_UPLINK:
        {
            auto proto_uplink = proto_spec->mutable_uplinkspec();
            proto_uplink->set_portid(api_spec->uplink_info.port.id, PDS_MAX_KEY_LEN);
        }
        break;
    case PDS_IF_TYPE_L3:
        {
            auto proto_l3 = proto_spec->mutable_l3ifspec();
            proto_l3->set_vpcid(api_spec->l3_if_info.vpc.id, PDS_MAX_KEY_LEN);
            proto_l3->set_portid(api_spec->l3_if_info.port.id, PDS_MAX_KEY_LEN);
            proto_l3->set_macaddress(
                      MAC_TO_UINT64(api_spec->l3_if_info.mac_addr));
            pds_encap_to_proto_encap(proto_l3->mutable_encap(),
                                     &api_spec->l3_if_info.encap);
            auto af = api_spec->l3_if_info.ip_prefix.addr.af;
            if (af == IP_AF_IPV4 || af == IP_AF_IPV6) {
                ippfx_api_spec_to_proto_spec(proto_l3->mutable_prefix(),
                                             &api_spec->l3_if_info.ip_prefix);
            }
        }
        break;
    case PDS_IF_TYPE_LOOPBACK:
        {
            auto proto_loopback = proto_spec->mutable_loopbackifspec();
            auto af = api_spec->loopback_if_info.ip_prefix.addr.af;
            if (af == IP_AF_IPV4 || af == IP_AF_IPV6) {
                ippfx_api_spec_to_proto_spec(proto_loopback->mutable_prefix(),
                                             &api_spec->loopback_if_info.ip_prefix);
            }
        }
        break;
    case PDS_IF_TYPE_CONTROL:
        {
            auto proto_control_if = proto_spec->mutable_controlifspec();
            auto af = api_spec->control_if_info.ip_prefix.addr.af;
            if (af == IP_AF_IPV4 || af == IP_AF_IPV6) {
                ippfx_api_spec_to_proto_spec(proto_control_if->mutable_prefix(),
                                             &api_spec->control_if_info.ip_prefix);
            }
            proto_control_if->set_macaddress(
                MAC_TO_UINT64(api_spec->control_if_info.mac_addr));
            if (api_spec->control_if_info.gateway.af == IP_AF_IPV4) {
                ipv4addr_api_spec_to_proto_spec(proto_control_if->mutable_gateway(),
                                                &api_spec->control_if_info.gateway.addr.v4_addr);
            } else {
                ipv6addr_api_spec_to_proto_spec(proto_control_if->mutable_gateway(),
                                                &api_spec->control_if_info.gateway.addr.v6_addr);
            }
        }
        break;
    default:
        break;
    }
}

static inline void
pds_if_api_status_to_proto (pds::InterfaceStatus *proto_status,
                            const pds_if_status_t *api_status,
                            pds_if_type_t type)
{
    proto_status->set_ifindex(api_status->ifindex);
    switch (type) {
    case PDS_IF_TYPE_UPLINK:
        {
            auto uplink_status = proto_status->mutable_uplinkifstatus();
            uplink_status->set_lifid(api_status->uplink_status.lif_id);
        }
        break;
    default:
        break;
    }
}

static inline void
pds_if_api_stats_to_proto (pds::InterfaceStats *proto_stats,
                           const pds_if_stats_t *api_stats)
{
}

static inline void
pds_if_api_info_to_proto (void *entry, void *ctxt)
{
    pds::InterfaceGetResponse *proto_rsp = (pds::InterfaceGetResponse *)ctxt;
    auto intf = proto_rsp->add_response();
    pds_if_info_t *info = (pds_if_info_t *)entry;

    pds_if_api_spec_to_proto(intf->mutable_spec(), &info->spec);
    pds_if_api_status_to_proto(intf->mutable_status(),
                               &info->status, info->spec.type);
    pds_if_api_stats_to_proto(intf->mutable_stats(), &info->stats);
}

static inline sdk_ret_t
pds_if_proto_to_api_spec (pds_if_spec_t *api_spec,
                          const pds::InterfaceSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->admin_state =
        proto_admin_state_to_pds_admin_state(proto_spec.adminstatus());
    switch (proto_spec.type()) {
    case pds::IF_TYPE_L3:
        if (proto_spec.txmirrorsessionid_size()) {
            PDS_TRACE_ERR("Tx Mirroring not supported on L3 interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if (proto_spec.rxmirrorsessionid_size()) {
            PDS_TRACE_ERR("Rx Mirroring not supported on L3 interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_IF_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&api_spec->l3_if_info.vpc,
                                      proto_spec.l3ifspec().vpcid());
        pds_obj_key_proto_to_api_spec(&api_spec->l3_if_info.port,
                                      proto_spec.l3ifspec().portid());
        api_spec->l3_if_info.encap =
            proto_encap_to_pds_encap(proto_spec.l3ifspec().encap());
        MAC_UINT64_TO_ADDR(api_spec->l3_if_info.mac_addr,
                           proto_spec.l3ifspec().macaddress());
        ippfx_proto_spec_to_api_spec(&api_spec->l3_if_info.ip_prefix,
                                     proto_spec.l3ifspec().prefix());
        break;

    case pds::IF_TYPE_LOOPBACK:
        if (proto_spec.txmirrorsessionid_size()) {
            PDS_TRACE_ERR("Tx Mirroring not supported on loopback interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if (proto_spec.rxmirrorsessionid_size()) {
            PDS_TRACE_ERR("Rx Mirroring not supported on loopback interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_IF_TYPE_LOOPBACK;
        ippfx_proto_spec_to_api_spec(&api_spec->loopback_if_info.ip_prefix,
                                     proto_spec.loopbackifspec().prefix());
        break;

    case pds::IF_TYPE_UPLINK:
        pds_obj_key_proto_to_api_spec(&api_spec->uplink_info.port,
                                      proto_spec.uplinkspec().portid());
        break;

    case pds::IF_TYPE_CONTROL:
        if (proto_spec.txmirrorsessionid_size()) {
            PDS_TRACE_ERR("Tx Mirroring not supported on inband interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        if (proto_spec.rxmirrorsessionid_size()) {
            PDS_TRACE_ERR("Rx Mirroring not supported on inband interface {}",
                          api_spec->key.str());
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_IF_TYPE_CONTROL;
        ippfx_proto_spec_to_api_spec(&api_spec->control_if_info.ip_prefix,
                                     proto_spec.controlifspec().prefix());
        MAC_UINT64_TO_ADDR(api_spec->control_if_info.mac_addr,
                           proto_spec.controlifspec().macaddress());
        ipaddr_proto_spec_to_api_spec(&api_spec->control_if_info.gateway,
                                      proto_spec.controlifspec().gateway());
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_lif_api_status_to_proto (pds::LifStatus *proto_status,
                             const pds_lif_status_t *api_status)
{
    proto_status->set_name(api_status->name);
    proto_status->set_ifindex(api_status->ifindex);
    proto_status->set_status(pds_admin_state_to_proto_admin_state(api_status->state));
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_lif_api_spec_to_proto_spec (pds::LifSpec *proto_spec,
                                const pds_lif_spec_t *api_spec)
{
    sdk_ret_t ret;
    pds_if_info_t pinned_ifinfo = { 0 };

    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->pinned_ifidx != IFINDEX_INVALID) {
        ret = api::pds_if_read(&api_spec->pinned_ifidx, &pinned_ifinfo);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to find if for {}, err {}",
                          api_spec->pinned_ifidx, ret);
        } else {
            proto_spec->set_pinnedinterface(pinned_ifinfo.spec.key.id,
                                            PDS_MAX_KEY_LEN);
        }
    }
    proto_spec->set_type(pds_lif_type_to_proto_lif_type(api_spec->type));
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac));
    return SDK_RET_OK;
}

// populate proto buf from lif api spec
static inline void
pds_lif_api_info_to_proto (void *entry, void *ctxt)
{
    auto rsp = ((pds::LifGetResponse *)ctxt)->add_response();
    auto proto_spec = rsp->mutable_spec();
    auto proto_status = rsp->mutable_status();
    pds_lif_info_t *api_info = (pds_lif_info_t *)entry;

    pds_lif_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    pds_lif_api_status_to_proto(proto_status, &api_info->status);
}

#endif    //__AGENT_SVC_INTERFACE_SVC_HPP__
