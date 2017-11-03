#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/crypto_apis.hpp"
#include "nic/hal/pd/capri/capri_barco_asym_apis.hpp"


namespace hal {


/* Asym APIs */
hal_ret_t crypto_asym_api_ecc_point_mul(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;
    uint8_t             px[ECC_MAX_KEY_SIZE];
    uint8_t             py[ECC_MAX_KEY_SIZE];

    key_size = req.ecc_point_mul_fp().ecc_domain_params().keysize();

    switch (key_size) {
        case 32:
            ret = pd::capri_barco_asym_ecc_point_mul_p256(
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().p().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().n().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().g().x().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().g().y().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().a().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_domain_params().b().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_point().x().data(),
                    (uint8_t *)req.ecc_point_mul_fp().ecc_point().y().data(),
                    (uint8_t *)req.ecc_point_mul_fp().k().data(),
                    px,
                    py);
            if (ret == HAL_RET_OK) {
                resp->mutable_ecc_point_mul_fp()->mutable_q()->mutable_x()->assign(
                        (const char*)px, (size_t) key_size);
                resp->mutable_ecc_point_mul_fp()->mutable_q()->mutable_y()->assign(
                        (const char*)py, (size_t) key_size);
            }
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}


hal_ret_t crypto_asym_api_ecdsa_sig_gen(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;
    uint8_t             r[ECC_MAX_KEY_SIZE];
    uint8_t             s[ECC_MAX_KEY_SIZE];

    key_size = req.ecdsa_sig_gen_fp().ecc_domain_params().keysize();

    switch (key_size) {
        case 32:
            ret = pd::capri_barco_asym_ecdsa_p256_sig_gen(
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().p().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().n().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().g().x().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().g().y().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().a().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().ecc_domain_params().b().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().da().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().k().data(),
                    (uint8_t *)req.ecdsa_sig_gen_fp().h().data(),
                    r,
                    s);
            if (ret == HAL_RET_OK) {
                resp->mutable_ecdsa_sig_gen_fp()->mutable_r()->assign(
                        (const char*)r, (size_t) key_size);
                resp->mutable_ecdsa_sig_gen_fp()->mutable_s()->assign(
                        (const char*)s, (size_t) key_size);
            }
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t crypto_asym_api_ecdsa_sig_verify(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;

    key_size = req.ecdsa_sig_verify_fp().ecc_domain_params().keysize();

    switch (key_size) {
        case 32:
            ret = pd::capri_barco_asym_ecdsa_p256_sig_verify(
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().p().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().n().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().g().x().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().g().y().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().a().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().ecc_domain_params().b().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().q().x().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().q().y().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().r().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().s().data(),
                    (uint8_t *)req.ecdsa_sig_verify_fp().h().data()
                    );
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t crypto_asym_api_rsa_encrypt(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;
    uint8_t             cipher_text[RSA_MAX_KEY_SIZE];

    key_size = req.rsa_encrypt().keysize();

    switch (key_size) {
        case 256:
            ret = pd::capri_barco_asym_rsa2k_encrypt(
                    (uint8_t *)req.rsa_encrypt().mod_n().data(),
                    (uint8_t *)req.rsa_encrypt().e().data(),
                    (uint8_t *)req.rsa_encrypt().plain_text().data(),
                    cipher_text
                    );
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->mutable_rsa_encrypt()->mutable_cipher_text()->assign(
                (const char*)cipher_text, (size_t) key_size);
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t crypto_asym_api_rsa_decrypt(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;
    uint8_t             plain_text[RSA_MAX_KEY_SIZE];

    key_size = req.rsa_decrypt().keysize();

    switch (key_size) {
        case 256:
            ret = pd::capri_barco_asym_rsa2k_decrypt(
                    (uint8_t *)req.rsa_decrypt().mod_n().data(),
                    (uint8_t *)req.rsa_decrypt().d().data(),
                    (uint8_t *)req.rsa_decrypt().cipher_text().data(),
                    plain_text
                    );
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->mutable_rsa_decrypt()->mutable_plain_text()->assign(
                (const char*)plain_text, (size_t) key_size);
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t crypto_asym_api_rsa_crt_decrypt(cryptoapis::CryptoApiRequest &req,
        cryptoapis::CryptoApiResponse *resp)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            key_size;
    uint8_t             plain_text[RSA_MAX_KEY_SIZE];

    key_size = req.rsa_crt_decrypt().keysize();

    switch (key_size) {
        case 256:
            ret = pd::capri_barco_asym_rsa2k_crt_decrypt(
                    (uint8_t *)req.rsa_crt_decrypt().p().data(),
                    (uint8_t *)req.rsa_crt_decrypt().q().data(),
                    (uint8_t *)req.rsa_crt_decrypt().dp().data(),
                    (uint8_t *)req.rsa_crt_decrypt().dq().data(),
                    (uint8_t *)req.rsa_crt_decrypt().qinv().data(),
                    (uint8_t *)req.rsa_crt_decrypt().cipher_text().data(),
                    plain_text
                    );
            break;
        default:
            HAL_TRACE_ERR("Unsupported key size: {}", key_size);
            ret = HAL_RET_INVALID_ARG;
            break;
    }
    if (ret == HAL_RET_OK) {
        resp->mutable_rsa_crt_decrypt()->mutable_plain_text()->assign(
                (const char*)plain_text, (size_t) key_size);
        resp->set_api_status(types::API_STATUS_OK);
    }
    else {
        resp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t crypto_api_invoke(cryptoapis::CryptoApiRequest &req, 
        cryptoapis::CryptoApiResponse *resp)
{

    hal_ret_t           ret = HAL_RET_OK;

    switch (req.api_type()) {
        case cryptoapis::ASYMAPI_ECC_POINT_MUL_FP:
            ret = crypto_asym_api_ecc_point_mul(req, resp);
            break;
        case cryptoapis::ASYMAPI_ECDSA_SIG_GEN_FP:
            ret = crypto_asym_api_ecdsa_sig_gen(req, resp);
            break;
        case cryptoapis::ASYMAPI_ECDSA_SIG_VERIFY_FP:
            ret = crypto_asym_api_ecdsa_sig_verify(req, resp);
            break;
        case cryptoapis::ASYMAPI_RSA_ENCRYPT:
            ret = crypto_asym_api_rsa_encrypt(req, resp);
            break;
        case cryptoapis::ASYMAPI_RSA_DECRYPT:
            ret = crypto_asym_api_rsa_decrypt(req, resp);
            break;
        case cryptoapis::ASYMAPI_RSA_CRT_DECRYPT:
            ret = crypto_asym_api_rsa_crt_decrypt(req, resp);
            break;
        default:
            HAL_TRACE_ERR("Invalid API: {}", req.api_type());
    }
    return ret;
}

} /* hal */
