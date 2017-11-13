#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/barco_rings_svc.hpp"
#include "nic/hal/src/barco_rings.hpp"


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

