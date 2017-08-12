#ifndef __PROXY_SVC_HPP__
#define __PROXY_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <proxy.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using proxy::Proxy;
using proxy::ProxySpec;
using proxy::ProxyStatus;
using proxy::ProxyResponse;
using proxy::ProxyKeyHandle;
using proxy::ProxyRequestMsg;
using proxy::ProxyResponseMsg;
using proxy::ProxyDeleteRequestMsg;
using proxy::ProxyDeleteResponseMsg;
using proxy::ProxyGetRequest;
using proxy::ProxyGetRequestMsg;
using proxy::ProxyGetResponse;
using proxy::ProxyGetResponseMsg;

class ProxyServiceImpl final : public Proxy::Service {
public:
    Status ProxyCreate(ServerContext *context,
                       const ProxyRequestMsg *req,
                       ProxyResponseMsg *rsp) override;

    Status ProxyUpdate(ServerContext *context,
                       const ProxyRequestMsg *req,
                       ProxyResponseMsg *rsp) override;


    Status ProxyDelete(ServerContext *context,
                       const ProxyDeleteRequestMsg *req,
                       ProxyDeleteResponseMsg *rsp) override;

    Status ProxyGet(ServerContext *context,
                    const ProxyGetRequestMsg *req,
                    ProxyGetResponseMsg *rsp) override;
};

#endif    // __PROXY_SVC_HPP__

