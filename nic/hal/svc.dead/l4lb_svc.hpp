#ifndef __HAL_SVC_HPP__
#define __HAL_SVC_HPP__

//------------------------------------------------------------------------------
// TODO: break up this file so that the binary size will be small
//       (multiple .cc files are including this whole thing now)
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/l4lb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using l4lb::L4Lb;
using l4lb::L4LbServiceRequestMsg;
using l4lb::L4LbServiceResponseMsg;
using l4lb::L4LbServiceDeleteRequestMsg;
using l4lb::L4LbServiceDeleteResponseMsg;
using l4lb::L4LbServiceGetRequestMsg;
using l4lb::L4LbServiceGetResponseMsg;
using l4lb::L4LbBackendRequestMsg;
using l4lb::L4LbBackendResponseMsg;
using l4lb::L4LbBackendDeleteRequestMsg;
using l4lb::L4LbBackendDeleteResponseMsg;
using l4lb::L4LbBackendGetRequestMsg;
using l4lb::L4LbBackendGetResponseMsg;

class L4LbServiceImpl final : public L4Lb::Service {
public:
    Status L4LbServiceCreate(ServerContext *context,
                             const L4LbServiceRequestMsg *req,
                             L4LbServiceResponseMsg *rsp) override;

    Status L4LbServiceUpdate(ServerContext *context,
                             const L4LbServiceRequestMsg *req,
                             L4LbServiceResponseMsg *rsp) override;


    Status L4LbServiceDelete(ServerContext *context,
                             const L4LbServiceDeleteRequestMsg *req,
                             L4LbServiceDeleteResponseMsg *rsp) override;

    Status L4LbServiceGet(ServerContext *context,
                          const L4LbServiceGetRequestMsg *req,
                          L4LbServiceGetResponseMsg *rsp) override;

    Status L4LbBackendCreate(ServerContext *context,
                             const L4LbBackendRequestMsg *req,
                             L4LbBackendResponseMsg *rsp) override;

    Status L4LbBackendUpdate(ServerContext *context,
                             const L4LbBackendRequestMsg *req,
                             L4LbBackendResponseMsg *rsp) override;


    Status L4LbBackendDelete(ServerContext *context,
                             const L4LbBackendDeleteRequestMsg *req,
                             L4LbBackendDeleteResponseMsg *rsp) override;

    Status L4LbBackendGet(ServerContext *context,
                          const L4LbBackendGetRequestMsg *req,
                          L4LbBackendGetResponseMsg *rsp) override;
};

#endif    // __HAL_SVC_HPP__

