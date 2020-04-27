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
    proto_status->set_description(api_status->description);
    proto_status->set_vendorid(api_status->vendor_id);
    switch (api_status->chip_type) {
    case sdk::platform::asic_type_t::SDK_ASIC_TYPE_CAPRI:
        proto_status->set_chiptype(types::ASIC_TYPE_CAPRI);
        break;
    case sdk::platform::asic_type_t::SDK_ASIC_TYPE_ELBA:
        proto_status->set_chiptype(types::ASIC_TYPE_ELBA);
        break;
    default:
        proto_status->set_chiptype(types::ASIC_TYPE_NONE);
        break;
    }
    proto_status->set_hardwarerevision(api_status->hardware_revision);
    proto_status->set_cpuvendor(api_status->cpu_vendor);
    proto_status->set_cpuspecification(api_status->cpu_specification);
    proto_status->set_socosversion(api_status->soc_os_version);
    proto_status->set_socdisksize(api_status->soc_disk_size);
    proto_status->set_pciespecification(api_status->pcie_specification);
    proto_status->set_pciebusinfo(api_status->pcie_bus_info);
    proto_status->set_numpcieports(api_status->num_pcie_ports);
    proto_status->set_numports(api_status->num_ports);
    proto_status->set_vendorname(api_status->vendor_name);
    proto_status->set_pxeversion(api_status->pxe_version);
    proto_status->set_uefiversion(api_status->uefi_version);
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

sdk_ret_t pds_svc_device_create(const pds::DeviceRequest *proto_req,
                                pds::DeviceResponse *proto_rsp);
sdk_ret_t pds_svc_device_update(const pds::DeviceRequest *proto_req,
                                pds::DeviceResponse *proto_rsp);
sdk_ret_t pds_svc_device_delete(const pds::DeviceDeleteRequest *proto_req,
                                pds::DeviceDeleteResponse *proto_rsp);
sdk_ret_t pds_svc_device_get(const pds::DeviceGetRequest *proto_req,
                             pds::DeviceGetResponse *proto_rsp);

static inline sdk_ret_t
pds_svc_device_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_DEVICE_CREATE:
        {
            pds::DeviceRequest req;
            pds::DeviceResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_device_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_DEVICE_UPDATE:
        {
            pds::DeviceRequest req;
            pds::DeviceResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_device_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_DEVICE_DELETE:
        {
            pds::DeviceDeleteRequest req;
            pds::DeviceDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_device_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_DEVICE_GET:
        {
            pds::DeviceGetRequest req;
            pds::DeviceGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_device_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_DEVICE_SVC_HPP__
