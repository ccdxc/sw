#include <openssl/rsa.h>
#include "dole_rsa.hpp"
#include "utils.hpp"

namespace dole {

static RSA_METHOD       *rsa_methods;
static int              rsa_ex_data_index = -1;

static int rsa_priv_enc(int flen,
                    const unsigned char *from,
                    unsigned char *to,
                    RSA *rsa,
                    int padding);
static int rsa_pub_dec(int flen,
                       const unsigned char *from,
                       unsigned char *to,
                       RSA *rsa,
                       int padding);
static RSA *local_rsa_alloc(dole_if::dole_key_t *key,
                            void *callback_data);
static int local_rsa_free(RSA *rsa);
static rsa_ex_data_t *local_rsa_ex_data_get(const RSA *rsa);

RSA_METHOD *
rsa_methods_get(void) 
{
    const RSA_METHOD    *pkcs1_methods = RSA_PKCS1_OpenSSL();

    if (rsa_methods) {
        return rsa_methods; 
    }

    rsa_methods = RSA_meth_new("dole RSA offload", 0);
    if (!rsa_methods) {
        DOLE_FUNC_ERR("Failed to malloc RSA_METHOD");
        return NULL;
    }

    // encrypt
    RSA_meth_set_pub_enc(rsa_methods, RSA_meth_get_pub_enc(pkcs1_methods));
    // verify arbitrary data
    RSA_meth_set_pub_dec(rsa_methods, rsa_pub_dec);
    // sign arbitrary data
    RSA_meth_set_priv_enc(rsa_methods, rsa_priv_enc);
    // decrypt
    RSA_meth_set_priv_dec(rsa_methods, RSA_meth_get_priv_dec(pkcs1_methods));
    // compute r0 = r0 ^ I mod rsa->n (May be NULL for some implementations)
    RSA_meth_set_mod_exp(rsa_methods, RSA_meth_get_mod_exp(pkcs1_methods));
    // compute r = a ^ p mod m (May be NULL for some implementations)
    RSA_meth_set_bn_mod_exp(rsa_methods, RSA_meth_get_bn_mod_exp(pkcs1_methods));
    // called at RSA_free()
    RSA_meth_set_finish(rsa_methods, local_rsa_free);

    if (rsa_ex_data_index < 0) {
        rsa_ex_data_index = RSA_get_ex_new_index(0, (void *)"dole RSA ex_data",
                                                 nullptr, nullptr, nullptr);
        DOLE_FUNC_INFO("Received RSA ex data index: {}", rsa_ex_data_index);
    }
    return rsa_methods;
}


void
rsa_methods_free(void) 
{
    if (rsa_methods) {
        RSA_meth_free(rsa_methods);
        rsa_methods = nullptr;
        if (rsa_ex_data_index != -1) {
            CRYPTO_free_ex_index(CRYPTO_EX_INDEX_RSA, rsa_ex_data_index);
            rsa_ex_data_index = -1;
        }
    }    
}


/* 
 * Initialize an RSA structure with an EVP pkey.
 * Note that the modulus n and key d_e must first be converted to
 * proper BIGNUM format as required by RSA standards.
 */
EVP_PKEY * 
rsa_pkey_load(ENGINE *engine,
              dole_if::dole_key_t *key,
              UI_METHOD *ui_method,
              void *callback_data)
{
    EVP_PKEY    *pkey = NULL;
    RSA         *rsa = NULL;

    rsa = local_rsa_alloc(key, callback_data);
    if (!rsa) {
        DOLE_FUNC_ERR("Failed to allocate RSA for pkey");
        goto error;
    }

    pkey = EVP_PKEY_new();
    if (!pkey) {
        DOLE_FUNC_ERR("Failed to allocate pkey");
        goto error;
    }
    
    EVP_PKEY_set1_RSA(pkey, rsa);
    return pkey;

error:
    if (pkey) {
        EVP_PKEY_free(pkey);
    }
    if (rsa) {
        local_rsa_free(rsa);
        RSA_free(rsa);
    }
    return NULL;
}

/*
 * Low level implementation of RSA private key encryption (possibly for signing)
 */
static int
rsa_priv_enc(int flen,
             const unsigned char *from,
             unsigned char *to,
             RSA *rsa,
             int padding)
{
    dp_mem_t            *digest_padded;
    dp_mem_t            *sig_actual;
    crypto_rsa::rsa_hw_sign_params_t hw_params;
    crypto_rsa::rsa_t   *crypto_rsa;
    rsa_ex_data_t       *ex_data;
    int                 ossl_ret;
    int                 rsa_len;

    DOLE_FUNC_DEBUG("enter from {:#x} flen {} to {:#x} padding {}",
                    (uint64_t)from, flen, (uint64_t)to, padding);
    if (!from || !to || !rsa) {
        DOLE_FUNC_ERR("Invalid args");
        goto error;
    }

    // Get Hw Key index
    ex_data = local_rsa_ex_data_get(rsa);
    if (!ex_data) {
        DOLE_FUNC_ERR("Failed to get rsa ex data");
        goto error;
    }
    crypto_rsa = static_cast<crypto_rsa::rsa_t *>(ex_data->caller_ctx());
    rsa_len = RSA_size(rsa);
    digest_padded = ex_data->digest_padded();
    DOLE_FUNC_DEBUG("flen {} rsa_len {} padding {} key_idx {}",
                    flen, rsa_len, padding, ex_data->key_idx());
    DOLE_FUNC_DEBUG("digest_padded mem_type {} line_size {} content_size {}",
                    digest_padded->mem_type_get(), digest_padded->line_size_get(),
                    digest_padded->content_size_get());
    assert((int)digest_padded->line_size_get() >= rsa_len);

    switch (padding) {

    case RSA_PKCS1_PADDING:

        /*
         * - Apply the EMSA-PKCS1-v1_5 encoding operation to the message M
         *    to produce an encoded message EM of length k octets:
         * - Convert the encoded message EM to an integer message
         *   representative m (stored in digest_padded).
         */
        ossl_ret = RSA_padding_add_PKCS1_type_1(digest_padded->read(),
                                                rsa_len, from, flen);
        break;

    case RSA_X931_PADDING:
        ossl_ret = RSA_padding_add_X931(digest_padded->read(),
                                        rsa_len, from, flen);
        break;

    case RSA_NO_PADDING:
        ossl_ret = RSA_padding_add_none(digest_padded->read(),
                                        rsa_len, from, flen);
        break;

    default:
        RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        DOLE_FUNC_ERR("Invalid padding type {}", padding);
        goto error;
    }

    if (ossl_ret <= 0) {
        DOLE_FUNC_ERR("Failed to add padding {}", padding);
        goto error;
    }

    digest_padded->content_size_set(rsa_len);
    digest_padded->write_thru();
    if (DOLE_IS_LOG_LEVEL_DEBUG()) {
        DOLE_FUNC_DEBUG("digest_padded");
        utils::dump(digest_padded->read(), rsa_len);
    }

    sig_actual = static_cast<dp_mem_t *>((void *)to);

    DOLE_FUNC_DEBUG("sig_actual mem_type {} line_size {} content_size {}",
                    sig_actual->mem_type_get(), sig_actual->line_size_get(),
                    sig_actual->content_size_get());
    /*
     * - Apply the RSASP1 signature primitive to the RSA private key K
     *   and the message representative m to produce an integer signature
     *   representative s:
     *      s = RSASP1 (K, m).
     * - Convert the signature representative s to a signature S of
     *   length k octets
     * - Output the signature S.
     *
     * The above steps are offloaded to HW, with the private key K
     * already programmed.
     */
    if (!crypto_rsa->push(hw_params.hash_input(digest_padded).
                                    sig_output(sig_actual))) {
        DOLE_FUNC_ERR("Failed HW push");
        goto error;
    }

    if (ex_data->wait_for_completion()) {
        crypto_rsa->post_push();
        if (!crypto_rsa->completion_check()) {
            DOLE_FUNC_ERR("Failed crypto_rsa HW op");
            goto error;
        }
    }

    /*
     * Note: if we didn't do wait_for_completion, then it means the caller
     * would have some other means to verify the result at a later time,
     * so we can return success here to satisfy Openssl.
     */
    return rsa_len;

error:
    return 0;
}


/*
 * Low level implementation of RSA public key decryption (possibly for 
 * signature verification).
 */
static int
rsa_pub_dec(int flen,
            const unsigned char *from,
            unsigned char *to,
            RSA *rsa,
            int padding)
{
    dp_mem_t            *digest_padded;
    dp_mem_t            *sig_expected;
    crypto_rsa::rsa_hw_enc_params_t hw_params;
    crypto_rsa::rsa_t   *crypto_rsa;
    rsa_ex_data_t       *ex_data;
    int                 rsa_len;

    DOLE_FUNC_DEBUG("enter from {:#x} flen {} to {:#x} padding {}",
                    (uint64_t)from, flen, (uint64_t)to, padding);
    if (!from || !to || !rsa) {
        DOLE_FUNC_ERR("Invalid args");
        goto error;
    }

    /* 
     * Per Openssl EVP_PKEY_verify() usage, we need to apply a primitive to
     * key(n, e) and the signature representative s to produce a message 
     * representative m (which is digest_padded).
     *
     * This function is called from Openssl int_rsa_verify() due to
     * the fact that we didn't fill out the method for RSA verify.
     *
     * key(n, e) is already programmed in HW, and s is denoted by 'from'.
     */
    ex_data = local_rsa_ex_data_get(rsa);
    if (!ex_data) {
        DOLE_FUNC_ERR("Failed to get rsa ex data");
        goto error;
    }
    crypto_rsa = static_cast<crypto_rsa::rsa_t *>(ex_data->caller_ctx());
    rsa_len = RSA_size(rsa);
    DOLE_FUNC_DEBUG("rsa_len {} key_idx {}", rsa_len, padding,
                    ex_data->key_idx());
    sig_expected = static_cast<dp_mem_t *>((void *)from);
    DOLE_FUNC_DEBUG("sig_expected mem_type {} line_size {} content_size {}",
                    sig_expected->mem_type_get(), sig_expected->line_size_get(),
                    sig_expected->content_size_get());
    /*
     * m produced from HW will be a 'padded' digest message
     */
    digest_padded = ex_data->digest_padded();
    DOLE_FUNC_DEBUG("digest_padded mem_type {} line_size {} content_size {}",
                    digest_padded->mem_type_get(), digest_padded->line_size_get(),
                    digest_padded->content_size_get());
    assert((int)digest_padded->line_size_get() >= rsa_len);

    digest_padded->content_size_set(rsa_len);
    if (!crypto_rsa->push(hw_params.plain_input(sig_expected).
                                    ciphered_output(digest_padded))) {
        DOLE_FUNC_ERR("Failed HW push");
        goto error;
    }

    if (ex_data->wait_for_completion()) {
        crypto_rsa->post_push();

        if (!crypto_rsa->completion_check()) {
            DOLE_FUNC_ERR("Failed crypto_rsa HW op");
            goto error;
        }

        if (DOLE_IS_LOG_LEVEL_DEBUG()) {
            DOLE_FUNC_DEBUG("digest_padded");
            utils::dump(digest_padded->read_thru(), rsa_len);
        }

        /*
         * Strip the padding prior to returning to Openssl int_rsa_verify()
         */
        switch (padding) {

        case RSA_PKCS1_PADDING:
            rsa_len = RSA_padding_check_PKCS1_type_1(to, rsa_len, 
                                  digest_padded->read_thru(), rsa_len, rsa_len);
            break;

        case RSA_X931_PADDING:
            rsa_len = RSA_padding_check_X931(to, rsa_len,
                                  digest_padded->read_thru(), rsa_len, rsa_len);
            break;

        case RSA_NO_PADDING:
            memcpy(to, digest_padded->read_thru(), rsa_len);
            break;

        default:
            RSAerr(RSA_F_RSA_NULL_PUBLIC_DECRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
            DOLE_FUNC_ERR("Invalid padding type {}", padding);
            goto error;
        }

        if (rsa_len <= 0) {
            goto error;
        }
    }

    /*
     * Note: if we didn't do wait_for_completion, then it means the caller
     * would have some other means to verify the result at a later time,
     * so we can return success here to satisfy Openssl.
     */
    return rsa_len;

error:
    return 0;
}

/*
 * Interface to openssl RSA structure
 */
static RSA *
local_rsa_alloc(dole_if::dole_key_t *key,
                void *callback_data)
{
    RSA                 *rsa = NULL;
    rsa_ex_data_t       *ex_data = NULL;
    BIGNUM              *bn_n = NULL;
    BIGNUM              *bn_d_e = NULL;
    dp_mem_t            *n;
    dp_mem_t            *d_e;

    rsa = RSA_new();
    if (!rsa) {
        DOLE_FUNC_ERR("Failed to allocate RSA");
        goto error;
    }

    /*
     * RSA_set_method() can cause a premature invocation to local_rsa_free()
     * so make sure to do it before setting any ex_data with RSA_set_ex_data().
     */
    RSA_set_method(rsa, rsa_methods_get()); 

    ex_data = new rsa_ex_data_t;
    if (!ex_data) {
        DOLE_FUNC_ERR("Failed to allocate RSA ex data");
        goto error;
    }

    n = key->n();
    d_e = key->d_e();
    ex_data->key_idx(key->key_idx()).
             digest_padded(key->digest_padded()).
             caller_ctx(callback_data).
             wait_for_completion(key->wait_for_completion());

    DOLE_FUNC_DEBUG("Setting up RSA ex_data");
    if (RSA_set_ex_data(rsa, rsa_ex_data_index, (void *)ex_data) <= 0) {
        DOLE_FUNC_ERR("Failed to set RSA ex data");
        goto error;
    }

    /*
     * Note that HW key had already been programmed so technically
     * we don't really need n/d/e here. However, the RSA structure
     * must at least know their lengths hence they are included here.
     *
     * Another assumption here is the HW key had been programmed
     * with the modulus n and d/e properly padded.
     */
    bn_n = BN_bin2bn(n->read(), n->content_size_get(), NULL);
    if (!bn_n) {
        DOLE_FUNC_ERR("Failed BN_bin2bn for n");
        goto error;
    }

    bn_d_e = BN_bin2bn(d_e->read(), d_e->content_size_get(), NULL);
    if (!bn_d_e) {
        DOLE_FUNC_ERR("Failed BN_bin2bn for d_e");
        goto error;
    }

    RSA_set0_key(rsa, bn_n, bn_d_e, NULL);
    return rsa;

error:
    if (bn_d_e) {
        BN_clear_free(bn_d_e);
    }
    if (bn_n) {
        BN_clear_free(bn_n);
    }
    if (rsa) {
        local_rsa_free(rsa);
        RSA_free(rsa);
    }
    return NULL;
}


static int
local_rsa_free(RSA *rsa)
{
    rsa_ex_data_t *ex_data = local_rsa_ex_data_get(rsa);
    if (ex_data) {
        DOLE_FUNC_DEBUG("Deleting RSA ex_data");
        delete ex_data;
        RSA_set_ex_data(rsa, rsa_ex_data_index, NULL);
    }

    /*
     * Caller (i.e., RSA_free()) will free rsa itself as well as
     * its set0_key fields such as e/n/d.
     */
    return 1;
}

static rsa_ex_data_t *
local_rsa_ex_data_get(const RSA *rsa)
{
    return static_cast<rsa_ex_data_t *>(RSA_get_ex_data(rsa, rsa_ex_data_index));
}

} // namespace dole
