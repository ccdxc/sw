//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/api/debug.hpp"

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

Status
MappingSvcImpl::MappingDump(ServerContext *context,
                            const pds::MappingDumpRequest *proto_req,
                            Empty *proto_rsp) {
    debug::cmd_ctxt_t ctxt = { 0 };

    ctxt.fd = debug::fd_get_from_cid(proto_req->cid());
    ctxt.cmd = debug::CLI_CMD_MAPPING_DUMP;

    if (proto_req->id_size()) {
        // If filter specified include in cmd args
        debug::cmd_args_t args = { 0 };
        debug::mapping_dump_args_t mapping_args = { 0 };

        mapping_args.key.vpc.id = proto_req->id(0).vpcid();
        ipaddr_proto_spec_to_api_spec(&mapping_args.key.ip_addr,
                                      proto_req->id(0).ipaddr());
        args.mapping_dump = &mapping_args;
        ctxt.args = &args;
    }

    debug::pds_mapping_dump(&ctxt);

    // Close File and reset state
    close(ctxt.fd);
    debug::cid_remove_from_fd_map(proto_req->cid());

    return Status::OK;
}
