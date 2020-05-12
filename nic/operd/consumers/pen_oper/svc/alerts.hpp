//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#ifndef __PEN_OPER_SVC_ALERTS_HPP__
#define __PEN_OPER_SVC_ALERTS_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/alerts.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerWriter;

using types::Empty;

using operd::AlertsSvc;
using operd::AlertsGetResponse;

class AlertsSvcImpl final : public AlertsSvc::Service {
public:
    Status AlertsGet(ServerContext *context, const Empty *req,
                     ServerWriter<AlertsGetResponse>* stream)
        override;
};

void publish_alert(const operd::Alert *alert);

#endif // __PEN_OPER_SVC_ALERTS_HPP__
