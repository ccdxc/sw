//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for device object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_DEVICE_SVC_HPP__
#define __AGENT_SVC_DEVICE_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/device.hpp"

// populate proto buf spec from device API spec
static inline void
pds_device_api_spec_to_proto (pds::DeviceSpec *proto_spec,
                              const pds_device_spec_t *api_spec)
{
    if ((api_spec->device_ip_addr.af == IP_AF_IPV4) ||
        (api_spec->device_ip_addr.af == IP_AF_IPV6)) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_ipaddr(),
                                      &api_spec->device_ip_addr);
    }
    proto_spec->set_macaddr(MAC_TO_UINT64(api_spec->device_mac_addr));
    if ((api_spec->gateway_ip_addr.af == IP_AF_IPV4) ||
        (api_spec->gateway_ip_addr.af == IP_AF_IPV6)) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_gatewayip(),
                                      &api_spec->gateway_ip_addr);
    }
    proto_spec->set_bridgingen(api_spec->bridging_en);
    proto_spec->set_learningen(api_spec->learning_en);
    proto_spec->set_learnagetimeout(api_spec->learn_age_timeout);
    proto_spec->set_overlayroutingen(api_spec->overlay_routing_en);
    switch (api_spec->dev_oper_mode) {
    case PDS_DEV_OPER_MODE_BITW:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_BITW);
        break;
    case PDS_DEV_OPER_MODE_HOST:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_HOST);
        break;
    default:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_NONE);
        break;
    }
    switch (api_spec->device_profile) {
    case PDS_DEVICE_PROFILE_2PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_2PF);
        break;
    case PDS_DEVICE_PROFILE_3PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_3PF);
        break;
    case PDS_DEVICE_PROFILE_4PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_4PF);
        break;
    case PDS_DEVICE_PROFILE_5PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_5PF);
        break;
    case PDS_DEVICE_PROFILE_6PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_6PF);
        break;
    case PDS_DEVICE_PROFILE_7PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_7PF);
        break;
    case PDS_DEVICE_PROFILE_8PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_8PF);
        break;
    default:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_DEFAULT);
        break;
    }
    switch (api_spec->memory_profile) {
    case PDS_MEMORY_PROFILE_DEFAULT:
    default:
        proto_spec->set_memoryprofile(pds::MEMORY_PROFILE_DEFAULT);
        break;
    }
}

// populate proto buf status from device API status
static inline void
pds_device_api_status_to_proto (pds::DeviceStatus *proto_status,
                                const pds_device_status_t *api_status)
{
    proto_status->set_systemmacaddress(MAC_TO_UINT64(api_status->fru_mac));
    proto_status->set_memory(api_status->memory_cap);
    proto_status->set_sku(api_status->part_num);
    proto_status->set_serialnumber(api_status->serial_num);
    proto_status->set_manufacturingdate(api_status->mnfg_date);
    proto_status->set_productname(api_status->product_name);
}

// populate proto buf stats from device API stats
static inline void
pds_device_api_stats_to_proto (pds::DeviceStats *proto_stats,
                               const pds_device_stats_t *api_stats)
{
    uint32_t i;
    pds::DeviceStatsEntry *entry;

    for (i = 0; i < api_stats->ing_drop_stats_count; i++) {
        entry = proto_stats->add_ingress();
        entry->set_name(api_stats->ing_drop_stats[i].name);
        entry->set_count(api_stats->ing_drop_stats[i].count);
    }

    for (i = 0; i < api_stats->egr_drop_stats_count; i++) {
        entry = proto_stats->add_egress();
        entry->set_name(api_stats->egr_drop_stats[i].name);
        entry->set_count(api_stats->egr_drop_stats[i].count);
    }
}

static inline sdk_ret_t
pds_device_proto_to_api_spec (pds_device_spec_t *api_spec,
                              const pds::DeviceSpec &proto_spec)
{
    types::IPAddress ipaddr = proto_spec.ipaddr();
    types::IPAddress gwipaddr = proto_spec.gatewayip();
    uint64_t macaddr = proto_spec.macaddr();

    memset(api_spec, 0, sizeof(pds_device_spec_t));
    if ((ipaddr.af() == types::IP_AF_INET) ||
        (ipaddr.af() == types::IP_AF_INET6)) {
        ipaddr_proto_spec_to_api_spec(&api_spec->device_ip_addr, ipaddr);
    }
    MAC_UINT64_TO_ADDR(api_spec->device_mac_addr, macaddr);
    if ((gwipaddr.af() == types::IP_AF_INET) ||
        (gwipaddr.af() == types::IP_AF_INET6)) {
        ipaddr_proto_spec_to_api_spec(&api_spec->gateway_ip_addr, gwipaddr);
    }
    api_spec->bridging_en = proto_spec.bridgingen();
    api_spec->learning_en = proto_spec.learningen();
    api_spec->learn_age_timeout = proto_spec.learnagetimeout();
    api_spec->overlay_routing_en = proto_spec.overlayroutingen();
    switch (proto_spec.devopermode()) {
    case pds::DEVICE_OPER_MODE_BITW:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_BITW;
        break;
    case pds::DEVICE_OPER_MODE_HOST:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_HOST;
        break;
    default:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_NONE;
        break;
    }
    switch (proto_spec.deviceprofile()) {
    case pds::DEVICE_PROFILE_2PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_2PF;
        break;
    case pds::DEVICE_PROFILE_3PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_3PF;
        break;
    case pds::DEVICE_PROFILE_4PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_4PF;
        break;
    case pds::DEVICE_PROFILE_5PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_5PF;
        break;
    case pds::DEVICE_PROFILE_6PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_6PF;
        break;
    case pds::DEVICE_PROFILE_7PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_7PF;
        break;
    case pds::DEVICE_PROFILE_8PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_8PF;
        break;
    default:
        api_spec->device_profile = PDS_DEVICE_PROFILE_DEFAULT;
        break;
    }
    switch (proto_spec.memoryprofile()) {
    case pds::MEMORY_PROFILE_DEFAULT:
    default:
        api_spec->memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
        break;
    }
    return SDK_RET_OK;
}


#endif    //__AGENT_SVC_DEVICE_SVC_HPP__
