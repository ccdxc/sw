#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/crypto_keys_svc.hpp"
#include "nic/hal/src/crypto_keys.hpp"

Status CryptoKeyServiceImpl::CryptoKeyCreate(ServerContext* context,
                const CryptoKeyCreateRequestMsg* request,
                CryptoKeyCreateResponseMsg* response)
{
    uint32_t                    idx, nreqs = request->request_size();
    CryptoKeyCreateResponse     *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        CryptoKeyCreateRequest  req = request->request(idx);

        hal::crypto_key_create(req, resp);

        //resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}

Status CryptoKeyServiceImpl::CryptoKeyRead(ServerContext* context,
                const CryptoKeyReadRequestMsg* request,
                CryptoKeyReadResponseMsg* response)
{
    uint32_t                    idx, nreqs = request->request_size();
    CryptoKeyReadResponse       *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        CryptoKeyReadRequest    req = request->request(idx);

        hal::crypto_key_read(req, resp);

        //resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}

Status CryptoKeyServiceImpl::CryptoKeyUpdate(ServerContext* context,
                const CryptoKeyUpdateRequestMsg* request,
                CryptoKeyUpdateResponseMsg* response)
{
    uint32_t                    idx, nreqs = request->request_size();
    CryptoKeyUpdateResponse     *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        CryptoKeyUpdateRequest      req = request->request(idx);

        hal::crypto_key_update(req, resp);

        //resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}

Status CryptoKeyServiceImpl::CryptoKeyDelete(ServerContext* context,
                const CryptoKeyDeleteRequestMsg* request,
                CryptoKeyDeleteResponseMsg* response)
{
    uint32_t                    idx, nreqs = request->request_size();
    CryptoKeyDeleteResponse     *resp;

    for (idx = 0; idx < nreqs; idx++) {
        resp = response->add_response();

        CryptoKeyDeleteRequest      req = request->request(idx);

        hal::crypto_key_delete(req, resp);

        //resp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}
