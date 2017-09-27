#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/descriptor_aol_svc.hpp"
#include "nic/hal/src/descriptor_aol.hpp"
#include "nic/hal/src/tcpcb.hpp"

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
