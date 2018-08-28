#ifndef __FWLOGSIM_SVC_HPP__
#define __FWLOGSIM_SVC_HPP__
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/internal.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::Server;
using internal::LogFlowRequestMsg;
using internal::LogFlowResponseMsg;
using internal::Internal;
using fwlog::FWEvent;

void log_firew_event(const FWEvent& fwe);

class FWLogSimServiceImpl final : public Internal::Service {
public:
    Status  LogFlow(ServerContext* context, const LogFlowRequestMsg* request, LogFlowResponseMsg* response);
};
#endif
