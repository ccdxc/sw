//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_device.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/switch.hpp"

static inline void
pds_agent_device_api_spec_fill (const tpc::SwitchSpec *proto_spec,
                                pds_device_spec_t *api_spec)
{
    types::IPAddress ipaddr = proto_spec->ipaddr();
    types::IPAddress gatewayip = proto_spec->gatewayip();
    uint64_t macaddr = proto_spec->macaddr();

    memset(api_spec, 0, sizeof(pds_device_spec_t));

    if (types::IP_AF_INET == ipaddr.af()) {
        api_spec->switch_ip_addr = ipaddr.v4addr();
    }

    if (types::IP_AF_INET == gatewayip.af()) {
        api_spec->gateway_ip_addr = gatewayip.v4addr();
    }

    MAC_UINT64_TO_ADDR(api_spec->switch_mac_addr, macaddr);
}

Status
SwitchSvcImpl::SwitchCreate(ServerContext *context, const tpc::SwitchSpec *spec,
                            tpc::SwitchStatus *status) {
    pds_device_spec_t api_spec;
    sdk_ret_t ret;

    pds_agent_device_api_spec_fill(spec, &api_spec);

    ret = pds_device_create(&api_spec);
    if (ret != sdk::SDK_RET_OK)
        return Status::CANCELLED;

    return Status::OK;
}
