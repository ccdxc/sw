#include "eng_if.hpp"
#include "utils.hpp"
#include "crypto_rsa.hpp"

namespace eng_if {

/*
 * Establish an EVP public/private key for i/f with openssl.
 * This will invoke the PSE engine function registered with
 * ENGINE_set_load_privkey_function().
 */
static EVP_PKEY *
local_rsa_pkey_load(void *caller_ctx,
                    crypto_asym::key_idx_t key_idx,
                    dp_mem_t *n,
                    dp_mem_t *d_e,
                    dp_mem_t *digest_padded,
                    dp_mem_t *salt_val,
                    bool wait_for_completion)
{
    PSE_KEY             key = {0};
    EVP_PKEY            *pkey;

    /*
     * OpenSSL 1.1.0 does not support EVP_PKEY_RSA_PSS so use the
     * default EVP_PKEY_RSA. The salt_len, if any, will be set
     * using EVP_PKEY_CTX_set_rsa_pss_saltlen().
     */
    key.type = EVP_PKEY_RSA;
    key.u.rsa_key.sign_key_id = key_idx;
    key.u.rsa_key.decrypt_key_id = key_idx;
    key.u.rsa_key.rsa_n.len = n->content_size_get();
    key.u.rsa_key.rsa_n.data = n->read();
    key.u.rsa_key.rsa_e.len = d_e->content_size_get();
    key.u.rsa_key.rsa_e.data = d_e->read();

    key.u.rsa_key.offload.offload_method = &crypto_rsa::pse_rsa_offload_method;
    key.u.rsa_key.offload.digest_padded_mem = (PSE_OFFLOAD_MEM *)digest_padded;
    key.u.rsa_key.offload.salt_val = (PSE_OFFLOAD_MEM *)salt_val;
    key.u.rsa_key.offload.wait_for_completion = wait_for_completion;

    pkey = ENGINE_load_private_key(eng_if_engine, (const char *)&key,
                                   NULL, caller_ctx);
    if (!pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA key");
    }
    return pkey;
}


/*
 * Generate a signature, i.e., sign an already hashed message
 * given in msg_digest.
 */
bool
rsa_sign(rsa_sign_params_t& params)
{
    EVP_PKEY_CTX        *pkey_ctx = NULL;
    EVP_PKEY            *evp_pkey;
    size_t              siglen;
    int                 ossl_ret;
    int                 salt_len;
    dp_mem_t            *digest = params.digest();
    dp_mem_t            *sig_actual = params.sig_actual();
    bool                success = false;

    /* Setup key */
    evp_pkey = local_rsa_pkey_load(params.rsa(), params.key_idx(),
                                   params.n(), params.d_e(),
                                   params.digest_padded(),
                                   params.salt_val(),
                                   params.wait_for_completion());
    if (!evp_pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA evp_pkey");
        goto done;
    }

    pkey_ctx = EVP_PKEY_CTX_new(evp_pkey, NULL);
    if (!pkey_ctx) {
        OFFL_FUNC_ERR("Failed to allocate EVP_PKEY_CTX");
        goto done;
    }
    ossl_ret = EVP_PKEY_sign_init(pkey_ctx);
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to init EVP_PKEY_CTX: {}", ossl_ret);
        goto done;
    }

    ossl_ret = EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, params.pad_mode());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup padding scheme in EVP_PKEY_CTX: {}",
                      ossl_ret);
        goto done;
    }

    salt_len = params.salt_val() ? 
               params.salt_val()->content_size_get() : 0;
    if (salt_len) {
        ossl_ret = EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, salt_len);
        if (ossl_ret <= 0) {
            OFFL_FUNC_ERR("Failed to setup salt_len in EVP_PKEY_CTX: {}",
                          ossl_ret);
            goto done;
        }
    }

    ossl_ret = EVP_PKEY_CTX_set_signature_md(pkey_ctx, params.md());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup hash scheme in EVP_PKEY_CTX: {}",
                      ossl_ret);
        goto done;
    }

    /* 
     * Determine the size of the signature
     */
    ossl_ret = EVP_PKEY_sign(pkey_ctx, NULL, &siglen, digest->read_thru(),
                             digest->content_size_get());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to determine sig len: {}", ossl_ret);
        goto done;
    }

    OFFL_FUNC_DEBUG("siglen: {} msglen: {}", siglen,
                    digest->content_size_get());
    if (params.pad_mode() != RSA_PKCS1_PSS_PADDING) {
        sig_actual->content_size_set(siglen);
    }

    ossl_ret = EVP_PKEY_sign(pkey_ctx, (unsigned char *)sig_actual,
                             &siglen, digest->read(),
                             digest->content_size_get());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR_OR_DEBUG(params.failure_expected(),
                               "Failed to sign digest message: {}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (!success && !params.failure_expected()) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();
    if (evp_pkey) {
        EVP_PKEY_free(evp_pkey);
    }
    if (pkey_ctx) {
        EVP_PKEY_CTX_free(pkey_ctx);
    }
    return success;
}


/*
 * Verify a signature
 */
bool
rsa_verify(rsa_verify_params_t& params)
{
    EVP_PKEY_CTX        *pkey_ctx = NULL;
    EVP_PKEY            *evp_pkey;
    int                 ossl_ret;
    dp_mem_t            *digest = params.digest();
    dp_mem_t            *sig_expected = params.sig_expected();
    bool                success = false;

    /*
     * The EVP_PKEY_verify_init() function initializes a public key algorithm
     * context using key pkey for a signature verification operation.
     *
     * The EVP_PKEY_verify() function performs a public key verification
     * operation using ctx. The signature is specified using the sig and 
     * siglen parameters. The verified data (i.e. the data believed originally
     * signed) is specified using the tbs (to be signed) and tbslen parameters.
     */
    evp_pkey = local_rsa_pkey_load(params.rsa(), params.key_idx(),
                                   params.n(), params.d_e(),
                                   params.digest_padded(),
                                   params.salt_val(),
                                   params.wait_for_completion());
    if (!evp_pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA evp_pkey");
        goto done;
    }

    pkey_ctx = EVP_PKEY_CTX_new(evp_pkey, NULL);
    if (!pkey_ctx) {
        OFFL_FUNC_ERR("Failed to allocate EVP_PKEY_CTX");
        goto done;
    }
    ossl_ret = EVP_PKEY_verify_init(pkey_ctx);
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to init EVP_PKEY_CTX: {}", ossl_ret);
        goto done;
    }

    /*
     * For PSS, signature verification will use OpenSSL salt_len
     * auto recovery so we don't set the salt_len here.
     */
    ossl_ret = EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, params.pad_mode());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup padding scheme in EVP_PKEY_CTX: {}",
                      ossl_ret);
        goto done;
    }

    ossl_ret = EVP_PKEY_CTX_set_signature_md(pkey_ctx, params.md());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup hash scheme in EVP_PKEY_CTX: {}",
                      ossl_ret);
        goto done;
    }

    OFFL_FUNC_DEBUG("sig_expected: {:#x} digest: {:#x}",
                    (uint64_t)sig_expected, (uint64_t)digest);
    ossl_ret = EVP_PKEY_verify(pkey_ctx, (const unsigned char *)sig_expected,
                               sig_expected->content_size_get(),
                               digest->read(),
                               digest->content_size_get());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR_OR_DEBUG(params.failure_expected(),
                               "Failed to verify digest message: {}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (!success && !params.failure_expected()) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();
    if (evp_pkey) {
        EVP_PKEY_free(evp_pkey);
    }
    if (pkey_ctx) {
        EVP_PKEY_CTX_free(pkey_ctx);
    }
    return success;
}


} // namespace eng_intf
