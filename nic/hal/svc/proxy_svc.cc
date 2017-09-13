//------------------------------------------------------------------------------
// Proxy service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <proxy_svc.hpp>
#include <proxy.hpp>

Status
ProxyServiceImpl::ProxyEnable(ServerContext *context,
                              const ProxyRequestMsg *req,
                              ProxyResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    ProxyResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Proxy Enable Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxy_enable(spec, response);
    }
    return Status::OK;
}

Status
ProxyServiceImpl::ProxyUpdate(ServerContext *context,
                              const ProxyRequestMsg *req,
                              ProxyResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    ProxyResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Proxy Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxy_update(spec, response);
    }
    return Status::OK;
}


Status
ProxyServiceImpl::ProxyDisable(ServerContext *context,
                              const ProxyDisableRequestMsg *req,
                              ProxyDisableResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Proxy Delete Request");
    return Status::OK;
}

Status
ProxyServiceImpl::ProxyGet(ServerContext *context,
                           const ProxyGetRequestMsg *req,
                           ProxyGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxyGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Proxy Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::proxy_get(request, response);
    }
    return Status::OK;
}

Status
ProxyServiceImpl::ProxyFlowConfig(ServerContext *context,
                                  const ProxyFlowConfigRequestMsg *reqmsg,
                                  ProxyResponseMsg *rspmsg)
{
    uint32_t         i, nreqs = reqmsg->request_size();
    ProxyResponse    *response;
    hal_ret_t            ret;

    HAL_TRACE_DEBUG("Rcvd Proxy Flow Config Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rspmsg->add_response();
        auto request = reqmsg->request(i);
        ret = hal::proxy_flow_config(request, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
   
}

Status
ProxyServiceImpl::ProxyGetFlowInfo(ServerContext *context,
                                   const ProxyGetFlowInfoRequestMsg *reqmsg,
                                   ProxyGetFlowInfoResponseMsg *rspmsg)
{
    uint32_t                    i, nreqs = reqmsg->request_size();
    ProxyGetFlowInfoResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Proxy Flow Info Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_READ);
        response = rspmsg->add_response();
        auto request = reqmsg->request(i);
        hal::proxy_get_flow_info(request, response);
        hal::hal_cfg_db_close(true);
    }
    return Status::OK;
   
}
