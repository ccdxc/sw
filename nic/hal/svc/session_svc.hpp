#ifndef __SESSION_SVC_HPP__
#define __SESSION_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/session.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using session::FlowKeyL2;
using session::FlowKeyV4;
using session::FlowKeyV6;
using session::FlowKey;
using session::FlowData;
using session::FlowInfo;
using session::FlowSpec;
using session::ConnTrackInfo;
using session::Session;
using session::SessionSpec;
using session::SessionStatus;
using session::SessionResponse;
using session::SessionRequestMsg;
using session::SessionResponseMsg;
using session::SessionDeleteRequestMsg;
using session::SessionDeleteResponseMsg;
using session::SessionGetRequest;
using session::SessionGetRequestMsg;
using session::SessionGetResponse;
using session::SessionGetResponseMsg;

class SessionServiceImpl final : public Session::Service {
    Status SessionCreate(ServerContext *context,
                         const SessionRequestMsg *req,
                         SessionResponseMsg *rsp) override;

    Status SessionDelete(ServerContext *context,
                         const SessionDeleteRequestMsg *req,
                         SessionDeleteResponseMsg *rsp) override;

    Status SessionGet(ServerContext *context,
                      const SessionGetRequestMsg *req,
                      SessionGetResponseMsg *rsp) override;
public:
};

#endif    // __SESSION_SVC_HPP__

