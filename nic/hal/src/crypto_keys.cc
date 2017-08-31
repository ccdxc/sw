#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <crypto_keys_svc.hpp>
#include <pd_api.hpp>
#include <crypto_keys.hpp>

using std::string;

namespace hal {

hal_ret_t   crypto_key_create(cryptokey::CryptoKeyCreateRequest &request,
        cryptokey::CryptoKeyCreateResponse *response)
{
    hal_ret_t           ret = HAL_RET_OK;
    int32_t             key_idx = -1;

    ret = pd::pd_crypto_alloc_key(&key_idx);
    if ((ret != HAL_RET_OK) || (key_idx < 0)) {
        response->set_api_status(types::API_STATUS_OUT_OF_MEM);
    }
    else {
        response->set_keyindex((uint32_t) key_idx);
        response->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}

hal_ret_t   crypto_key_read(cryptokey::CryptoKeyReadRequest &request,
        cryptokey::CryptoKeyReadResponse *response)
{
    hal_ret_t                   ret = HAL_RET_OK;
    int32_t                     key_idx = -1;
    crypto_key_t                pd_key;
    string                      *key;

    key_idx = request.keyindex();
    if (key_idx >= 0) {
        ret = pd::pd_crypto_read_key(key_idx, &pd_key);
        if (ret != HAL_RET_OK) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
        }
        else {
            response->mutable_key()->set_keyindex((uint32_t)key_idx);
            response->mutable_key()->set_key_type(pd_key.key_type);
            response->mutable_key()->set_key_size(pd_key.key_size);
            key = response->mutable_key()->mutable_key();
            key->assign((const char*)pd_key.key, (size_t)pd_key.key_size);
            response->set_api_status(types::API_STATUS_OK);
        }
    }
    else {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
    }
    return ret;
}

hal_ret_t   crypto_key_update(cryptokey::CryptoKeyUpdateRequest &request,
        cryptokey::CryptoKeyUpdateResponse *response)
{
    hal_ret_t                   ret = HAL_RET_OK;
    int32_t                     key_idx = -1;
    crypto_key_t                pd_key;

    const CryptoKeySpec &key = request.key();
    key_idx = key.keyindex();

    if ((key_idx >= 0) && (key.key_size() <= CRYPTO_KEY_SIZE_MAX)) {
        pd_key.key_type = key.key_type();
        pd_key.key_size = key.key_size();
        memcpy((void*)pd_key.key, (void*) key.key().data(), pd_key.key_size);
        ret = pd::pd_crypto_write_key(key_idx, &pd_key);
        if (ret != HAL_RET_OK) {
            response->set_api_status(types::API_STATUS_ERR);
        }
        else {
            response->set_api_status(types::API_STATUS_OK);
        }
    }
    else {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
    }
    return ret;
}

hal_ret_t   crypto_key_delete(cryptokey::CryptoKeyDeleteRequest &request,
        cryptokey::CryptoKeyDeleteResponse *response)
{
    hal_ret_t                   ret = HAL_RET_OK;
    int32_t                     key_idx = -1;

    key_idx = request.keyindex();
    ret = pd::pd_crypto_free_key(key_idx);
    if (ret != HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
    }
    else {
        response->set_keyindex((uint32_t) key_idx);
        response->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}

}
