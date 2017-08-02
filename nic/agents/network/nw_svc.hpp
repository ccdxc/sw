#ifndef __NW_SVC_HPP__
#define __NW_SVC_HPP__

#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include "net.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;
using net::VlanRequest;
using net::VlanResponse;
using net::Net;

class NetServiceImpl final : public Net::Service {
public:
    Status VlanCreate(ServerContext *context, const VlanRequest *req,
                      VlanResponse *rsp) override;
    Status VlanUpdate(ServerContext *context, const VlanRequest *req,
                      VlanResponse *rsp) override;
};

#endif    // __NW_SVC_HPP__

