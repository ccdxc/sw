//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for dhcp object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_DHCP_SVC_HPP__
#define __AGENT_SVC_DHCP_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/dhcp.hpp"

static inline sdk_ret_t
pds_dhcp_relay_proto_to_api_spec (pds_dhcp_relay_spec_t *api_spec,
                                  const pds::DHCPRelaySpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());
    ipaddr_proto_spec_to_api_spec(&api_spec->server_ip, proto_spec.serverip());
    ipaddr_proto_spec_to_api_spec(&api_spec->agent_ip, proto_spec.agentip());
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_proto_to_api_spec (pds_dhcp_proxy_spec_t *api_spec,
                                  const pds::DHCPProxySpec &proto_spec)
{
    ipaddr_proto_spec_to_api_spec(&api_spec->server_ip, proto_spec.serverip());
    api_spec->mtu = proto_spec.mtu();
    ipaddr_proto_spec_to_api_spec(&api_spec->gateway_ip,
                                  proto_spec.gatewayip());
    ipaddr_proto_spec_to_api_spec(&api_spec->dns_server_ip,
                                  proto_spec.dnsserverip());
    ipaddr_proto_spec_to_api_spec(&api_spec->ntp_server_ip,
                                  proto_spec.ntpserverip());
    if (proto_spec.domainname().empty()) {
        api_spec->domain_name[0] = '\0';
    } else {
        strncpy(api_spec->domain_name, proto_spec.domainname().c_str(),
                PDS_MAX_DOMAIN_NAME_LEN);
        api_spec->domain_name[PDS_MAX_DOMAIN_NAME_LEN] = '\0';
    }
    api_spec->lease_timeout = proto_spec.leasetimeout();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_policy_proto_to_api_spec (pds_dhcp_policy_spec_t *api_spec,
                                   const pds::DHCPPolicySpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    if (proto_spec.has_relayspec()) {
        api_spec->type = PDS_DHCP_POLICY_TYPE_RELAY;
        pds_dhcp_relay_proto_to_api_spec(&api_spec->relay_spec,
                                         proto_spec.relayspec());
    } else if (proto_spec.has_proxyspec()) {
        api_spec->type = PDS_DHCP_POLICY_TYPE_PROXY;
        pds_dhcp_proxy_proto_to_api_spec(&api_spec->proxy_spec,
                                         proto_spec.proxyspec());
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_spec_to_proto (pds::DHCPRelaySpec *proto_spec,
                                  const pds_dhcp_relay_spec_t *api_spec)
{
    //proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_serverip(),
                                  &api_spec->server_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_agentip(),
                                  &api_spec->agent_ip);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_spec_to_proto (pds::DHCPProxySpec *proto_spec,
                                  const pds_dhcp_proxy_spec_t *api_spec)
{
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_serverip(),
                                  &api_spec->server_ip);
    proto_spec->set_mtu(api_spec->mtu);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_gatewayip(),
                                  &api_spec->gateway_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_dnsserverip(),
                                  &api_spec->dns_server_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_ntpserverip(),
                                  &api_spec->ntp_server_ip);
    proto_spec->set_domainname(api_spec->domain_name);
    proto_spec->set_leasetimeout(api_spec->lease_timeout);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_policy_api_spec_to_proto (pds::DHCPPolicySpec *proto_spec,
                                   const pds_dhcp_policy_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_DHCP_POLICY_TYPE_RELAY) {
        pds_dhcp_relay_api_spec_to_proto(proto_spec->mutable_relayspec(),
                                         &api_spec->relay_spec);
    } else if (api_spec->type == PDS_DHCP_POLICY_TYPE_PROXY) {
        pds_dhcp_proxy_api_spec_to_proto(proto_spec->mutable_proxyspec(),
                                         &api_spec->proxy_spec);
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_status_to_proto (pds::DHCPRelayStatus *proto_status,
                                    const pds_dhcp_relay_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_status_to_proto (pds::DHCPProxyStatus *proto_status,
                                    const pds_dhcp_proxy_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_stats_to_proto (pds::DHCPRelayStats *proto_stats,
                                   const pds_dhcp_relay_stats_t *api_stats)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_stats_to_proto (pds::DHCPProxyStats *proto_stats,
                                   const pds_dhcp_proxy_stats_t *api_stats)
{
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_dhcp_policy_api_info_to_proto (const pds_dhcp_policy_info_t *api_info,
                                  void *ctxt)
{
    pds::DHCPPolicyGetResponse *proto_rsp = (pds::DHCPPolicyGetResponse *)ctxt;
    auto dhcp = proto_rsp->add_response();
    pds::DHCPPolicySpec *proto_spec = dhcp->mutable_spec();
    pds::DHCPPolicyStatus *proto_status = dhcp->mutable_status();
    pds::DHCPPolicyStats *proto_stats = dhcp->mutable_stats();

    pds_dhcp_policy_api_spec_to_proto(proto_spec, &api_info->spec);
    if (api_info->spec.type == PDS_DHCP_POLICY_TYPE_RELAY) {
        pds_dhcp_relay_api_status_to_proto(proto_status->mutable_relaystatus(),
                                           &api_info->status.relay_status);
        pds_dhcp_relay_api_stats_to_proto(proto_stats->mutable_relaystats(),
                                          &api_info->stats.relay_stats);
    } else if (api_info->spec.type == PDS_DHCP_POLICY_TYPE_PROXY) {
        pds_dhcp_proxy_api_status_to_proto(proto_status->mutable_proxystatus(),
                                           &api_info->status.proxy_status);
        pds_dhcp_proxy_api_stats_to_proto(proto_stats->mutable_proxystats(),
                                          &api_info->stats.proxy_stats);
    }
}

#endif    //__AGENT_SVC_DHCP_SVC_HPP__
