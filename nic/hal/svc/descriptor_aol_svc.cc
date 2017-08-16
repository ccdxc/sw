#include <base.h>
#include <trace.hpp>
#include <descriptor_aol_svc.hpp>
#include <descriptor_aol.hpp>
#include <tcpcb.hpp>

Status 
DescrAolServiceImpl::DescrAolGet(ServerContext* context,
                        const DescrAolRequestMsg* request,
                        DescrAolResponseMsg* response)
{
    uint32_t                idx, nreqs = request->request_size();
    DescrAolSpec            *resp;

//    HAL_TRACE_DEBUG("DescrAol Get Request nreqs:{}", nreqs);

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();
        DescrAolRequest  req = request->request(idx);

        hal::descriptor_aol_get(req, resp);

        resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}
