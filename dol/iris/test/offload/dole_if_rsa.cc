#include "dole_if.hpp"
#include "utils.hpp"

namespace dole_if {

/*
 * Establish an EVP public/private key for i/f with openssl.
 * This will invoke the DOLE function registered with
 * ENGINE_set_load_privkey_function() which, in turn, will
 * lead to dole::rsa_pkey_load().
 */
static EVP_PKEY *
local_rsa_pkey_load(void *caller_ctx,
                    crypto_asym::key_idx_t key_idx,
                    dp_mem_t *n,
                    dp_mem_t *d_e,
                    dp_mem_t *digest_padded,
                    bool wait_for_completion)
{
    dole_key_t          key;
    EVP_PKEY            *pkey;

    key.key_type(EVP_PKEY_RSA).
        key_idx(key_idx).
        n(n).
        d_e(d_e).
        digest_padded(digest_padded).
        wait_for_completion(wait_for_completion);

    pkey = ENGINE_load_private_key(dole, (const char *)&key,
                                   NULL, caller_ctx);
    if (!pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA key");
    }
    return pkey;
}


/*
 * Compute the hash digest for msg and store result in digest_output.
 */
const dole_evp_md_t *
rsa_digest(rsa_digest_params_t& params)
{
    EVP_MD_CTX          *md_ctx = NULL;
    const dole_evp_md_t *md = NULL;
    unsigned int        digest_size;
    dp_mem_t            *msg = params.msg();
    dp_mem_t            *digest = params.digest();

    /*
     * Using an OpenSSL message digest/hash function, consists of the
     * following steps:
     *
     * - Create a Message Digest context
     * - Initialise the context by identifying the algorithm to be used 
     *   (built-in algorithms are defined in evp.h)
     * - Provide the message whose digest needs to be calculated. Messages
     *   can be divided into sections and provided over a number of calls
     *   to the library if necessary
     * - Caclulate the digest
     * - Clean up the context if no longer required
     *
     * Message digest algorithms are identified using an EVP_MD object. 
     * These are built-in  to the library and obtained through appropriate
     * library calls (e.g. such as EVP_sha256() or EVP_sha512()).
     */
    md_ctx = EVP_MD_CTX_create();
    if (!md_ctx) {
        OFFL_FUNC_ERR("Failed to create EVP_MD_CTX");
        goto error;
    }

    md = hash_algo_find(params.hash_algo());
    if (!md) {
        OFFL_FUNC_ERR("Failed to find md for {}", params.hash_algo());
        goto error;
    }

    /* 
     * Identify the algorithm
     */
    if (EVP_DigestInit_ex(md_ctx, md, NULL) <= 0) {
        OFFL_FUNC_ERR("Failed to initialize EVP_MD_CTX");
        md = NULL;
        goto error;
    }

    /* 
     * Calculate the digest
     */
    if (EVP_DigestUpdate(md_ctx, msg->read(), msg->content_size_get()) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestUpdate size {}",
                      msg->content_size_get());
        md = NULL;
        goto error;
    }

    if (EVP_DigestFinal_ex(md_ctx, digest->read(), &digest_size) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestFinal_ex");
        md = NULL;
        goto error;
    }

    OFFL_FUNC_DEBUG("digest input size {} output size {}",
                    msg->content_size_get(), digest_size);
    digest->content_size_set(digest_size);
    digest->write_thru();

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        OFFL_FUNC_DEBUG("digest");
        utils::dump(digest->read(), digest->content_size_get());
    }

error:
    if (!md) {
        ERR_print_errors(dole_if_bio);
    }
    ERR_clear_error();
    if (md_ctx) {
        EVP_MD_CTX_destroy(md_ctx);
    }
    return md;
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
    dp_mem_t            *digest = params.digest();
    dp_mem_t            *sig_actual = params.sig_actual();
    bool                success = false;

    /* Setup key */
    evp_pkey = local_rsa_pkey_load(params.rsa(), params.key_idx(),
                                   params.n(), params.d_e(),
                                   params.digest_padded(),
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
    sig_actual->content_size_set(siglen);

    ossl_ret = EVP_PKEY_sign(pkey_ctx, (unsigned char *)sig_actual,
                             &siglen, digest->read(),
                             digest->content_size_get());
    if ((ossl_ret <= 0) && !params.failure_expected()) {
        OFFL_FUNC_ERR("Failed to sign digest message: {}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (!success && !params.failure_expected()) {
        ERR_print_errors(dole_if_bio);
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
    if ((ossl_ret <= 0) && !params.failure_expected()) {
        OFFL_FUNC_ERR("Failed to verify digest message: {}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (!success && !params.failure_expected()) {
        ERR_print_errors(dole_if_bio);
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


} // namespace dole_intf
