#include "dole_if.hpp"

namespace dole_if {

static EVP_PKEY *
local_rsa_pkey_load(void *user_ctx,
                    crypto_asym::key_idx_t key_idx,
                    dp_mem_t *n,
                    dp_mem_t *d_e);

/*
 * Compute the hash digest for msg and store result in digest_output.
 */
const dole_evp_md_t *
rsa_msg_digest_push(const string& hash_algo,
                    dp_mem_t *msg,
                    dp_mem_t *digest_output)
{
    EVP_MD_CTX          *md_ctx = NULL;
    const dole_evp_md_t *md = NULL;
    unsigned int        digest_size;

    md_ctx = EVP_MD_CTX_create();
    if (!md_ctx) {
        OFFL_FUNC_ERR("Failed to create EVP_MD_CTX");
        goto error;
    }

    md = hash_algo_find(hash_algo);
    if (!md) {
        OFFL_FUNC_ERR("Failed to find md for {}", hash_algo);
        goto error;
    }

    if (EVP_DigestInit_ex(md_ctx, md, NULL) <= 0) {
        OFFL_FUNC_ERR("Failed to initialize EVP_MD_CTX");
        md = NULL;
        goto error;
    }

    if (EVP_DigestUpdate(md_ctx, msg->read(), msg->content_size_get()) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestUpdate size {}",
                      msg->content_size_get());
        md = NULL;
        goto error;
    }

    if (EVP_DigestFinal_ex(md_ctx, digest_output->read(), &digest_size) <= 0) {
        OFFL_FUNC_ERR("Failed EVP_DigestFinal_ex");
        md = NULL;
        goto error;
    }

    OFFL_FUNC_DEBUG("digest input size {} output size {}",
                    msg->content_size_get(), digest_size);
    digest_output->content_size_set(digest_size);
    digest_output->write_thru();

error:
    if (md_ctx) {
        EVP_MD_CTX_destroy(md_ctx);
    }
    return md;
}

/*
 * Generate or verify a signature
 */
bool
rsa_sig_push(void *user_ctx,
             const dole_evp_md_t *md,
             crypto_asym::key_idx_t key_idx,
             dp_mem_t *n,
             dp_mem_t *d_e,
             dp_mem_t *msg_digest,
             dp_mem_t *sig_actual)
{
    EVP_PKEY_CTX        *pkey_ctx = NULL;
    EVP_PKEY            *evp_pkey;
    size_t              siglen;
    int                 ossl_ret;
    bool                success = false;

    /* Setup key */
    evp_pkey = local_rsa_pkey_load(user_ctx, key_idx, n, d_e);
    if (!evp_pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA evp_pkey");
        goto done;
    }

    pkey_ctx = EVP_PKEY_CTX_new(evp_pkey, NULL);
    if (!pkey_ctx) {
        OFFL_FUNC_ERR("Failed to allocate EVP_PKEY_CTX");
        ERR_print_errors(dole_bio);
        goto done;
    }
    ossl_ret = EVP_PKEY_sign_init(pkey_ctx);
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to init EVP_PKEY_CTX, ret:{}", ossl_ret);
        goto done;
    }

    ossl_ret = EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PADDING);
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup padding scheme in EVP_PKEY_CTX, ret:{}",
                      ossl_ret);
        goto done;
    }

    ossl_ret = EVP_PKEY_CTX_set_signature_md(pkey_ctx, md);
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to setup hash scheme in EVP_PKEY_CTX, ret:{}",
                      ossl_ret);
        goto done;
    }

    /* Determine buffer length */
    ossl_ret = EVP_PKEY_sign(pkey_ctx, NULL, &siglen, msg_digest->read_thru(),
                             msg_digest->content_size_get());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to determine sig len, ret:{}", ossl_ret);
        goto done;
    }
    //OFFL_FUNC_DEBUG("siglen: {}", siglen);
    sig_actual->content_size_set(siglen);

    ossl_ret = EVP_PKEY_sign(pkey_ctx, (unsigned char *)sig_actual,
                             &siglen, msg_digest->read(),
                             msg_digest->content_size_get());
    if (ossl_ret <= 0) {
        OFFL_FUNC_ERR("Failed to generate sig, ret:{}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (evp_pkey) {
        EVP_PKEY_free(evp_pkey);
    }
    if (pkey_ctx) {
        EVP_PKEY_CTX_free(pkey_ctx);
    }
    return success;
}


/*
 * Establish an EVP public/private key for i/f with openssl.
 * This will invoke the DOLE function registered with
 * ENGINE_set_load_privkey_function() which, in turn, will
 * lead to dole::rsa_pkey_load().
 */
static EVP_PKEY *
local_rsa_pkey_load(void *user_ctx,
                    crypto_asym::key_idx_t key_idx,
                    dp_mem_t *n,
                    dp_mem_t *d_e)
{
    dole_key_t          key;
    EVP_PKEY            *pkey;

    key.key_type(EVP_PKEY_RSA).
        key_idx(key_idx).
        n(n).
        d_e(d_e);
    pkey = ENGINE_load_private_key(dole, (const char *)&key,
                                   NULL, user_ctx);
    if (!pkey) {
        OFFL_FUNC_ERR("Failed to setup RSA key");
    }
    return pkey;
}

} // namespace dole_intf
