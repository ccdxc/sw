#ifndef __PROXYRCB_SVC_HPP__
#define __PROXYRCB_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/proxyrcb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using proxyrcb::ProxyrCb;
using proxyrcb::ProxyrCbSpec;
using proxyrcb::ProxyrCbStatus;
using proxyrcb::ProxyrCbResponse;
using proxyrcb::ProxyrCbKeyHandle;
using proxyrcb::ProxyrCbRequestMsg;
using proxyrcb::ProxyrCbResponseMsg;
using proxyrcb::ProxyrCbDeleteRequestMsg;
using proxyrcb::ProxyrCbDeleteResponseMsg;
using proxyrcb::ProxyrCbGetRequest;
using proxyrcb::ProxyrCbGetRequestMsg;
using proxyrcb::ProxyrCbGetResponse;
using proxyrcb::ProxyrCbGetResponseMsg;

class ProxyrCbServiceImpl final : public ProxyrCb::Service {
public:
    Status ProxyrCbCreate(ServerContext *context,
                          const ProxyrCbRequestMsg *req,
                          ProxyrCbResponseMsg *rsp) override;

    Status ProxyrCbUpdate(ServerContext *context,
                          const ProxyrCbRequestMsg *req,
                          ProxyrCbResponseMsg *rsp) override;


    Status ProxyrCbDelete(ServerContext *context,
                          const ProxyrCbDeleteRequestMsg *req,
                          ProxyrCbDeleteResponseMsg *rsp) override;

    Status ProxyrCbGet(ServerContext *context,
                       const ProxyrCbGetRequestMsg *req,
                       ProxyrCbGetResponseMsg *rsp) override;
};

#endif    // __PROXYRCB_SVC_HPP__

