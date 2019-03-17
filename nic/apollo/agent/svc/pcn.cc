//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/pcn.hpp"

// Build PCN API spec from protobuf spec
static inline void
pds_agent_pcn_api_spec_fill (const pds::PCNSpec &proto_spec,
                             pds_vcn_spec_t *api_spec)
{
    pds::PCNType type;

    api_spec->key.id = proto_spec.id();
    type = proto_spec.type();
    if (type == pds::PCN_TYPE_TENANT) {
        api_spec->type = PDS_VCN_TYPE_TENANT;
    } else if (type == pds::PCN_TYPE_SUBSTRATE) {
        api_spec->type = PDS_VCN_TYPE_SUBSTRATE;
    }
    pds_agent_util_ip_pfx_fill(proto_spec.prefix(), &api_spec->pfx);
}

Status
PCNSvcImpl::PCNCreate(ServerContext *context, const pds::PCNRequest *proto_req,
                      pds::PCNResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_vcn_spec_t api_spec = {0};

            pds_agent_pcn_api_spec_fill(proto_req->request(i), &api_spec);
            if (pds_vcn_create(&api_spec) == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }
    return Status::CANCELLED;
}
