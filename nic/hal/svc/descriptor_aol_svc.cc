#include <base.h>
#include <trace.hpp>
#include <descriptor_aol_svc.hpp>

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
//        HAL_TRACE_DEBUG("DescrAol Get Handle:{}", req.descr_aol_handle());
        /* TODO: Link to HAL PD */
        resp->set_descr_aol_handle(req.descr_aol_handle());
        resp->set_address1(0x0a0b0c0d0a0b0c0d);
        resp->set_offset1(0x01020304);
        resp->set_length1(0x05060708);
//        HAL_TRACE_DEBUG("A:{} O:{} L:{}", resp->address1(), resp->offset1(), resp->length1());
        resp->set_address2(0x0a0b0c0d0a0b0c0d);
        resp->set_offset2(0x01020304);
        resp->set_length2(0x05060708);
//        HAL_TRACE_DEBUG("A:{} O:{} L:{}", resp->address2(), resp->offset2(), resp->length2());
        resp->set_address3(0x0a0b0c0d0a0b0c0d);
        resp->set_offset3(0x01020304);
        resp->set_length3(0x05060708);
//        HAL_TRACE_DEBUG("A:{} O:{} L:{}", resp->address3(), resp->offset3(), resp->length3());
        resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}
