//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/test/flow_test/flow_test.hpp"

#ifdef PDS_FLOW_TEST
extern flow_test *g_flow_test_obj;
#endif

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i++) {
            pds_local_mapping_spec_t local_spec = { 0 };
            pds_remote_mapping_spec_t remote_spec = { 0 };

            if (proto_req->request(i).tunnelid() == 0) {
                pds_agent_local_mapping_api_spec_fill(&local_spec,
                                                      proto_req->request(i));
#ifdef PDS_FLOW_TEST
                g_flow_test_obj->add_local_ep(local_spec.key.vpc.id,
                                              local_spec.key.ip_addr);
#endif
            } else {
                pds_agent_remote_mapping_api_spec_fill(&remote_spec,
                                                       proto_req->request(i));
#ifdef PDS_FLOW_TEST
                g_flow_test_obj->add_remote_ep(remote_spec.key.vpc.id,
                                               remote_spec.key.ip_addr);
#endif
            }

            if (!core::agent_state::state()->pds_mock_mode()) {
                if (proto_req->request(i).tunnelid() == 0) {
                    if (pds_local_mapping_create(&local_spec) != sdk:: SDK_RET_OK)
                        return Status::CANCELLED;
                } else {
                    if (pds_remote_mapping_create(&remote_spec) != sdk:: SDK_RET_OK)
                        return Status::CANCELLED;
                }
            }
        }
    }

    return Status::OK;
}

Status
MappingSvcImpl::MappingUpdate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i++) {
            pds_local_mapping_spec_t local_spec = { 0 };
            pds_remote_mapping_spec_t remote_spec = { 0 };

            if (proto_req->request(i).tunnelid() == 0) {
                pds_agent_local_mapping_api_spec_fill(&local_spec,
                                                      proto_req->request(i));
            } else {
                pds_agent_remote_mapping_api_spec_fill(&remote_spec,
                                                       proto_req->request(i));
            }

            if (!core::agent_state::state()->pds_mock_mode()) {
                if (proto_req->request(i).tunnelid() == 0) {
                    if (pds_local_mapping_update(&local_spec) != sdk:: SDK_RET_OK)
                        return Status::CANCELLED;
                } else {
                    if (pds_remote_mapping_update(&remote_spec) != sdk:: SDK_RET_OK)
                        return Status::CANCELLED;
                }
            }
        }
    }

    return Status::OK;
}
