// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_Nexthop_HPP__
#define __AGENT_SVC_Nexthop_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/nh.grpc.pb.h"
#include "nic/apollo/api/include/pds_nexthop.hpp"

using grpc::Status;
using grpc::ServerContext;

using pds::NhSvc;

typedef struct nh_get_all_args_s {
    pds_nh_type_t type;
    void *ctxt;
} nh_get_all_args_t;

class NhSvcImpl final : public NhSvc::Service {
public:
    Status NexthopCreate(ServerContext *context, const pds::NexthopRequest *req,
                         pds::NexthopResponse *rsp) override;
    Status NexthopUpdate(ServerContext *context, const pds::NexthopRequest *req,
                         pds::NexthopResponse *rsp) override;
    Status NexthopDelete(ServerContext *context,
                         const pds::NexthopDeleteRequest *proto_req,
                         pds::NexthopDeleteResponse *proto_rsp) override;
    Status NexthopGet(ServerContext *context,
                      const pds::NexthopGetRequest *proto_req,
                      pds::NexthopGetResponse *proto_rsp) override;
    Status NhGroupCreate(ServerContext *context, const pds::NhGroupRequest *req,
                         pds::NhGroupResponse *rsp) override;
    Status NhGroupUpdate(ServerContext *context, const pds::NhGroupRequest *req,
                         pds::NhGroupResponse *rsp) override;
    Status NhGroupDelete(ServerContext *context,
                         const pds::NhGroupDeleteRequest *proto_req,
                         pds::NhGroupDeleteResponse *proto_rsp) override;
    Status NhGroupGet(ServerContext *context,
                      const pds::NhGroupGetRequest *proto_req,
                      pds::NhGroupGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_Nexthop_HPP__
