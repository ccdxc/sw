//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/device.hpp"

static inline void
pds_agent_device_api_spec_fill (const pds::DeviceSpec &proto_spec,
                                pds_device_spec_t *api_spec)
{
    types::IPAddress ipaddr = proto_spec.ipaddr();
    types::IPAddress gatewayip = proto_spec.gatewayip();
    uint64_t macaddr = proto_spec.macaddr();

    memset(api_spec, 0, sizeof(pds_device_spec_t));

    if (types::IP_AF_INET == ipaddr.af()) {
        api_spec->device_ip_addr = ipaddr.v4addr();
    }

    if (types::IP_AF_INET == gatewayip.af()) {
        api_spec->gateway_ip_addr = gatewayip.v4addr();
    }

    MAC_UINT64_TO_ADDR(api_spec->device_mac_addr, macaddr);
}

Status
DeviceSvcImpl::DeviceCreate(ServerContext *context, const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;

    if (proto_req) {
        pds_device_spec_t api_spec = {0};
        pds_agent_device_api_spec_fill(proto_req->request(), &api_spec);
        ret = pds_device_create(&api_spec);
        if (ret != sdk::SDK_RET_OK)
            return Status::CANCELLED;
    }
    return Status::OK;
}
