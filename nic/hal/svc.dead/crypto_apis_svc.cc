#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/crypto_apis_svc.hpp"
#include "nic/hal/src/crypto_apis.hpp"


Status CryptoApisServiceImpl::CryptoApiInvoke(ServerContext* context,
        const CryptoApiRequestMsg* request,
        CryptoApiResponseMsg* response)
{

    uint32_t                    idx, nreqs = request->request_size();
    CryptoApiResponse           *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        CryptoApiRequest req = request->request(idx);

        hal::crypto_api_invoke(req, resp);

        //resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}
