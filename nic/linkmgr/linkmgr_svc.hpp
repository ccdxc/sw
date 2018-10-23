// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_SVC_HPP__
#define __LINKMGR_SVC_HPP__

#include "grpc++/grpc++.h"

#include "nic/include/base.hpp"

#include "gen/proto/types.pb.h"
#include "gen/proto/port.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using port::Port;

using port::PortRequestMsg;
using port::PortResponseMsg;
using port::PortGetRequestMsg;
using port::PortGetResponseMsg;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortInfoGetRequestMsg;
using port::PortInfoGetResponseMsg;

using port::PortSpec;
using port::PortResponse;
using port::PortGetRequest;
using port::PortGetResponse;
using port::PortDeleteRequest;
using port::PortDeleteResponse;
using port::PortInfoGetRequest;
using port::PortInfoGetResponse;

using port::PortStats;
using port::PortOperStatus;
using port::PortType;
using port::PortSpeed;
using port::PortAdminState;
using port::PortFecType;

using port::MacStatsType;

class PortServiceImpl final : public Port::Service {
public:
    Status PortCreate(ServerContext *context,
                      const PortRequestMsg *req,
                      PortResponseMsg *rsp) override;

    Status PortUpdate(ServerContext *context,
                      const PortRequestMsg *req,
                      PortResponseMsg *rsp) override;

    Status PortDelete(ServerContext *context,
                      const PortDeleteRequestMsg *req,
                      PortDeleteResponseMsg *rsp) override;

    Status PortGet(ServerContext *context,
                   const PortGetRequestMsg *req,
                   PortGetResponseMsg *rsp) override;

    Status PortInfoGet(ServerContext *context,
                       const PortInfoGetRequestMsg *req,
                       PortInfoGetResponseMsg *rsp) override;
};

using sdk::linkmgr::port_args_t;
void populate_port_create_args(PortSpec& spec, port_args_t *args);
bool validate_port_create(PortSpec& spec, PortResponse *response);
bool validate_port_update(PortSpec& spec, PortResponse*rsp);
void populate_port_update_args(PortSpec& spec, port_args_t *args);

#endif    // __LINKMGR_SVC_HPP__
