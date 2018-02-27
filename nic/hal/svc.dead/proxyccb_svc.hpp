#ifndef __PROXYCCB_SVC_HPP__
#define __PROXYCCB_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/proxyccb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using proxyccb::ProxycCb;
using proxyccb::ProxycCbSpec;
using proxyccb::ProxycCbStatus;
using proxyccb::ProxycCbResponse;
using proxyccb::ProxycCbKeyHandle;
using proxyccb::ProxycCbRequestMsg;
using proxyccb::ProxycCbResponseMsg;
using proxyccb::ProxycCbDeleteRequestMsg;
using proxyccb::ProxycCbDeleteResponseMsg;
using proxyccb::ProxycCbGetRequest;
using proxyccb::ProxycCbGetRequestMsg;
using proxyccb::ProxycCbGetResponse;
using proxyccb::ProxycCbGetResponseMsg;

class ProxycCbServiceImpl final : public ProxycCb::Service {
public:
    Status ProxycCbCreate(ServerContext *context,
                          const ProxycCbRequestMsg *req,
                          ProxycCbResponseMsg *rsp) override;

    Status ProxycCbUpdate(ServerContext *context,
                          const ProxycCbRequestMsg *req,
                          ProxycCbResponseMsg *rsp) override;


    Status ProxycCbDelete(ServerContext *context,
                          const ProxycCbDeleteRequestMsg *req,
                          ProxycCbDeleteResponseMsg *rsp) override;

    Status ProxycCbGet(ServerContext *context,
                       const ProxycCbGetRequestMsg *req,
                       ProxycCbGetResponseMsg *rsp) override;
};

#endif    // __PROXYCCB_SVC_HPP__

