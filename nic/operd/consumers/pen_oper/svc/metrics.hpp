//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#ifndef __PEN_OPER_SVC_METRICS_HPP__
#define __PEN_OPER_SVC_METRICS_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/metrics.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;

using operd::MetricsSvc;
using operd::MetricsGetRequest;
using operd::MetricsGetResponse;

class MetricsSvcImpl final : public MetricsSvc::Service {
public:
    Status MetricsGet(
        ServerContext* context,
        ServerReaderWriter<MetricsGetResponse, MetricsGetRequest>* stream)
        override;
};

#endif // __PEN_OPER_SVC_METRICS_HPP__
