#ifndef __ENDPOINT_SVC_HPP__
#define __ENDPOINT_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/endpoint.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using endpoint::Endpoint;
using endpoint::EndpointSpec;
using endpoint::EndpointStatus;
using endpoint::EndpointResponse;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using endpoint::EndpointGetRequest;
using endpoint::EndpointGetRequestMsg;
using endpoint::EndpointGetResponse;
using endpoint::EndpointGetResponseMsg;
using endpoint::EndpointUpdateRequestMsg;
using endpoint::EndpointUpdateResponseMsg;
using endpoint::EndpointUpdateRequest;
using endpoint::EndpointKeyHandle;


class EndpointServiceImpl final : public Endpoint::Service {
public:
    Status EndpointCreate(ServerContext *context,
                          const EndpointRequestMsg *req,
                          EndpointResponseMsg *rsp) override;

    Status EndpointUpdate(ServerContext *context,
                          const EndpointUpdateRequestMsg *req,
                          EndpointUpdateResponseMsg *rsp) override;


    Status EndpointDelete(ServerContext *context,
                          const EndpointDeleteRequestMsg *req,
                          EndpointDeleteResponseMsg *rsp) override;

    Status EndpointGet(ServerContext *context,
                       const EndpointGetRequestMsg *req,
                       EndpointGetResponseMsg *rsp) override;
};

#endif    // __ENDPOINT_SVC_HPP__

