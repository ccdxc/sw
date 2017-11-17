#ifndef __PROXY_SVC_HPP__
#define __PROXY_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/proxy.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using proxy::Proxy;
using proxy::ProxySpec;
using proxy::ProxyStatus;
using proxy::ProxyResponse;
using proxy::ProxyKeyHandle;
using proxy::ProxyRequestMsg;
using proxy::ProxyResponseMsg;
using proxy::ProxyDisableRequestMsg;
using proxy::ProxyDisableResponseMsg;
using proxy::ProxyGetRequest;
using proxy::ProxyGetRequestMsg;
using proxy::ProxyGetResponse;
using proxy::ProxyGetResponseMsg;
using proxy::ProxyFlowConfigRequestMsg;
using proxy::ProxyGetFlowInfoRequestMsg;
using proxy::ProxyGetFlowInfoResponseMsg;
using proxy::ProxyGetFlowInfoResponse;
using proxy::ProxyGlobalCfgRequestMsg;
using proxy::ProxyGlobalCfgResponseMsg;
using proxy::ProxyGlobalCfgRequest;

class ProxyServiceImpl final : public Proxy::Service {
public:
    Status ProxyEnable(ServerContext *context,
                       const ProxyRequestMsg *req,
                       ProxyResponseMsg *rsp) override;

    Status ProxyUpdate(ServerContext *context,
                       const ProxyRequestMsg *req,
                       ProxyResponseMsg *rsp) override;


    Status ProxyDisable(ServerContext *context,
                        const ProxyDisableRequestMsg *req,
                        ProxyDisableResponseMsg *rsp) override;

    Status ProxyGet(ServerContext *context,
                    const ProxyGetRequestMsg *req,
                    ProxyGetResponseMsg *rsp) override;

    Status ProxyFlowConfig(ServerContext *context,
                           const ProxyFlowConfigRequestMsg *reqmsg,
                           ProxyResponseMsg *rspmsg) override;

    Status ProxyGetFlowInfo(ServerContext *context,
                            const ProxyGetFlowInfoRequestMsg *reqmsg,
                            ProxyGetFlowInfoResponseMsg *rspmsg) override;

    Status ProxyGlobalCfg(ServerContext *context,
			  const ProxyGlobalCfgRequestMsg *reqmsg,
			  ProxyGlobalCfgResponseMsg *rspmsg) override;

};

#endif    // __PROXY_SVC_HPP__

