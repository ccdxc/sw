//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for vnic object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_VNIC_SVC_HPP__
#define __AGENT_SVC_VNIC_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"

// populate proto buf spec from vnic API spec
static inline void
pds_vnic_api_spec_to_proto (pds::VnicSpec *proto_spec,
                            const pds_vnic_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_hostname(api_spec->hostname);
    proto_spec->set_subnetid(api_spec->subnet.id, PDS_MAX_KEY_LEN);
    pds_encap_to_proto_encap(proto_spec->mutable_vnicencap(),
                             &api_spec->vnic_encap);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac_addr));
    //proto_spec->set_providermacaddress(
                        //MAC_TO_UINT64(api_spec->provider_mac_addr));
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_sourceguardenable(api_spec->binding_checks_en);
    proto_spec->set_v4meterid(api_spec->v4_meter.id, PDS_MAX_KEY_LEN);
    proto_spec->set_v6meterid(api_spec->v6_meter.id, PDS_MAX_KEY_LEN);
    if (api_spec->tx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->tx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_txmirrorsessionid(i + 1);
            }
        }
    }
    if (api_spec->rx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->rx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_rxmirrorsessionid(i + 1);
            }
        }
    }
    proto_spec->set_switchvnic(api_spec->switch_vnic);
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        proto_spec->add_ingv4securitypolicyid(api_spec->ing_v4_policy[i].id,
                                              PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        proto_spec->add_ingv6securitypolicyid(api_spec->ing_v6_policy[i].id,
                                              PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        proto_spec->add_egv4securitypolicyid(api_spec->egr_v4_policy[i].id,
                                             PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        proto_spec->add_egv6securitypolicyid(api_spec->egr_v6_policy[i].id,
                                             PDS_MAX_KEY_LEN);
    }
    proto_spec->set_hostif(api_spec->host_if.id, PDS_MAX_KEY_LEN);
    proto_spec->set_txpolicerid(api_spec->tx_policer.id, PDS_MAX_KEY_LEN);
    proto_spec->set_rxpolicerid(api_spec->rx_policer.id, PDS_MAX_KEY_LEN);
    proto_spec->set_primary(api_spec->primary);
    proto_spec->set_maxsessions(api_spec->max_sessions);
    proto_spec->set_flowlearnen(api_spec->flow_learn_en);
    proto_spec->set_meteren(api_spec->meter_en);
}

// populate proto buf status from vnic API status
static inline void
pds_vnic_api_status_to_proto (pds::VnicStatus *proto_status,
                              const pds_vnic_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from vnic API stats
static inline void
pds_vnic_api_stats_to_proto (pds::VnicStats *proto_stats,
                             const pds_vnic_stats_t *api_stats)
{
    proto_stats->set_txbytes(api_stats->tx_bytes);
    proto_stats->set_txpackets(api_stats->tx_pkts);
    proto_stats->set_rxbytes(api_stats->rx_bytes);
    proto_stats->set_rxpackets(api_stats->rx_pkts);
}

// populate proto buf from vnic API info
static inline void
pds_vnic_api_info_to_proto (pds_vnic_info_t *api_info, void *ctxt)
{
    pds::VnicGetResponse *proto_rsp = (pds::VnicGetResponse *)ctxt;
    auto vnic = proto_rsp->add_response();
    pds::VnicSpec *proto_spec = vnic->mutable_spec();
    pds::VnicStatus *proto_status = vnic->mutable_status();
    pds::VnicStats *proto_stats = vnic->mutable_stats();

    pds_vnic_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vnic_api_status_to_proto(proto_status, &api_info->status);
    pds_vnic_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build VNIC api spec from proto buf spec
static inline sdk_ret_t
pds_vnic_proto_to_api_spec (pds_vnic_spec_t *api_spec,
                            const pds::VnicSpec &proto_spec)
{
    uint32_t msid;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    if (proto_spec.hostname().empty()) {
        api_spec->hostname[0] = '\0';
    } else {
        strncpy(api_spec->hostname, proto_spec.hostname().c_str(),
                PDS_MAX_HOST_NAME_LEN);
         api_spec->hostname[PDS_MAX_HOST_NAME_LEN] = '\0';
    }
    pds_obj_key_proto_to_api_spec(&api_spec->subnet, proto_spec.subnetid());
    api_spec->vnic_encap = proto_encap_to_pds_encap(proto_spec.vnicencap());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->mac_addr, proto_spec.macaddress());
    api_spec->binding_checks_en = proto_spec.sourceguardenable();
    for (int i = 0; i < proto_spec.txmirrorsessionid_size(); i++) {
        msid = proto_spec.txmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid tx mirror session id {} in vnic {} spec, "
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->tx_mirror_session_bmap |= (1 << (msid - 1));
    }
    for (int i = 0; i < proto_spec.rxmirrorsessionid_size(); i++) {
        msid = proto_spec.rxmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid rx mirror session id {} in vnic {} spec",
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->rx_mirror_session_bmap |= (1 << (msid - 1));
    }
    pds_obj_key_proto_to_api_spec(&api_spec->v4_meter, proto_spec.v4meterid());
    pds_obj_key_proto_to_api_spec(&api_spec->v6_meter, proto_spec.v6meterid());
    api_spec->switch_vnic = proto_spec.switchvnic();
    if (proto_spec.ingv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v4_policy = proto_spec.ingv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v4_policy[i],
                                      proto_spec.ingv4securitypolicyid(i));
    }
    if (proto_spec.ingv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v6_policy = proto_spec.ingv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v6_policy[i],
                                      proto_spec.ingv6securitypolicyid(i));
    }
    if (proto_spec.egv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v4_policy = proto_spec.egv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v4_policy[i],
                                      proto_spec.egv4securitypolicyid(i));
    }
    if (proto_spec.egv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v6_policy = proto_spec.egv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v6_policy[i],
                                      proto_spec.egv6securitypolicyid(i));
    }
    pds_obj_key_proto_to_api_spec(&api_spec->host_if, proto_spec.hostif());
    pds_obj_key_proto_to_api_spec(&api_spec->tx_policer,
                                  proto_spec.txpolicerid());
    pds_obj_key_proto_to_api_spec(&api_spec->rx_policer,
                                  proto_spec.rxpolicerid());
    api_spec->primary = proto_spec.primary();
    api_spec->max_sessions = proto_spec.maxsessions();
    api_spec->flow_learn_en = proto_spec.flowlearnen();
    api_spec->meter_en = proto_spec.meteren();
    return SDK_RET_OK;
}

#endif    //__AGENT_SVC_VNIC_SVC_HPP__
