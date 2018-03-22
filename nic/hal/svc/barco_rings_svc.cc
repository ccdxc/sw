#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/barco_rings_svc.hpp"
#include "nic/hal/src/aclqos/barco_rings.hpp"


Status BarcoRingsServiceImpl::GetOpaqueTagAddr(ServerContext* context,
    const GetOpaqueTagAddrRequestMsg* request,
    GetOpaqueTagAddrResponseMsg* response)
{
    uint32_t                    idx, nreqs = request->request_size();
    GetOpaqueTagAddrResponse     *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        const GetOpaqueTagAddrRequest&  req = request->request(idx);

        hal::GetOpaqueTagAddr(req, resp);

    }
    return Status::OK;
}

Status
BarcoRingsServiceImpl::BarcoGetReqDescrEntry(ServerContext *context,
					     const BarcoGetReqDescrEntryRequestMsg *req,
					     BarcoGetReqDescrEntryResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    BarcoGetReqDescrEntryResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Barco Get Descriptor Request ");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::BarcoGetReqDescrEntry(request, response);
    }
    return Status::OK;
}

Status
BarcoRingsServiceImpl::BarcoGetRingMeta(ServerContext *context,
					const BarcoGetRingMetaRequestMsg *req,
					BarcoGetRingMetaResponseMsg *rsp)
{
    uint32_t                  i, nreqs = req->request_size();
    BarcoGetRingMetaResponse  *response;

    HAL_TRACE_DEBUG("Rcvd Barco Get Ring Meta Request ");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::BarcoGetRingMeta(request, response);
    }
    return Status::OK;
}
