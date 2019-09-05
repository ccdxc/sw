//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i++) {
            pds_local_mapping_spec_t local_spec = { 0 };
            pds_remote_mapping_spec_t remote_spec = { 0 };

            if (proto_req->request(i).has_tunnelip() == false) {
                pds_local_mapping_proto_to_api_spec(&local_spec,
                                                    proto_req->request(i));
                hooks::local_mapping_create(&local_spec);
            } else {
                pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                     proto_req->request(i));
                hooks::remote_mapping_create(&remote_spec);
            }

            if (!core::agent_state::state()->pds_mock_mode()) {
                if (proto_req->request(i).has_tunnelip() == false) {
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

            if (proto_req->request(i).has_tunnelip() == false) {
                pds_local_mapping_proto_to_api_spec(&local_spec,
                                                    proto_req->request(i));
            } else {
                pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                     proto_req->request(i));
            }

            if (!core::agent_state::state()->pds_mock_mode()) {
                if (proto_req->request(i).has_tunnelip() == false) {
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
