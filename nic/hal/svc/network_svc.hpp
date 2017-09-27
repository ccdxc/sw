#ifndef __NETWORK_SVC_HPP__
#define __NETWORK_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/nw.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using nw::Network;
using nw::NetworkSpec;
using nw::NetworkStatus;
using nw::NetworkResponse;
using nw::NetworkKeyHandle;
using nw::NetworkRequestMsg;
using nw::NetworkResponseMsg;
using nw::NetworkDeleteRequest;
using nw::NetworkDeleteRequestMsg;
using nw::NetworkDeleteResponseMsg;
using nw::NetworkGetRequest;
using nw::NetworkGetRequestMsg;
using nw::NetworkGetResponse;
using nw::NetworkGetResponseMsg;

class NetworkServiceImpl final : public Network::Service {
public:
    Status NetworkCreate(ServerContext *context,
                         const NetworkRequestMsg *req,
                         NetworkResponseMsg *rsp) override;

    Status NetworkUpdate(ServerContext *context,
                         const NetworkRequestMsg *req,
                         NetworkResponseMsg *rsp) override;


    Status NetworkDelete(ServerContext *context,
                         const NetworkDeleteRequestMsg *req,
                         NetworkDeleteResponseMsg *rsp) override;

    Status NetworkGet(ServerContext *context,
                      const NetworkGetRequestMsg *req,
                      NetworkGetResponseMsg *rsp) override;
};
#endif    // __NETWORK_SVC_HPP__

