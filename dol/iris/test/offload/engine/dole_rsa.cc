#include <openssl/rsa.h>
#include "dole_rsa.hpp"

namespace dole {

static RSA_METHOD       *rsa_methods;
static int              rsa_ex_data_index = -1;

static int rsa_sig_push(int flen,
                        const unsigned char *from,
                        unsigned char *to,
                        RSA *rsa,
                        int padding);
static RSA *local_rsa_alloc(dole_if::dole_key_t *key,
                            void *callback_data);
static int local_rsa_free(RSA *rsa);
static rsa_ex_data_t *local_rsa_ex_data_get(RSA *rsa);

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
    RSA_meth_set_pub_dec(rsa_methods, RSA_meth_get_pub_dec(pkcs1_methods));
    // sign arbitrary data
    RSA_meth_set_priv_enc(rsa_methods, rsa_sig_push);
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
 * Low level implementation of RSA sign
 */
static int
rsa_sig_push(int flen,
             const unsigned char *from,
             unsigned char *to,
             RSA *rsa,
             int padding)
{
    dp_mem_t            *digest_padded = NULL;
    dp_mem_t            *sig_actual;
    crypto_rsa::rsa_sig_padded_push_params_t padded_params;
    crypto_rsa::rsa_t   *crypto_rsa;
    rsa_ex_data_t       *ex_data;
    int                 ossl_ret;
    int                 rsa_len;

    DOLE_FUNC_DEBUG("enter");
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
    crypto_rsa = static_cast<crypto_rsa::rsa_t *>(ex_data->user_ctx());
    rsa_len = RSA_size(rsa);
    DOLE_FUNC_DEBUG("flen {} rsa_len {} padding {} key_idx {}", 
                    flen, rsa_len, padding, ex_data->key_idx());
    digest_padded = new dp_mem_t(1, rsa_len);
    switch (padding) {

    case RSA_PKCS1_PADDING:
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

    case RSA_SSLV23_PADDING:
    default:
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        goto error;
    }

    if (ossl_ret <= 0) {
        DOLE_FUNC_ERR("Failed to add padding {}", padding);
        goto error;
    }

    digest_padded->write_thru();
    sig_actual = static_cast<dp_mem_t *>((void *)to);

    DOLE_FUNC_DEBUG("digest_padded mem_type {} line_size {} content_size {}",
                    digest_padded->mem_type_get(), digest_padded->line_size_get(),
                    digest_padded->content_size_get());
    DOLE_FUNC_DEBUG("sig_actual mem_type {} line_size {} content_size {}",
                    sig_actual->mem_type_get(), sig_actual->line_size_get(),
                    sig_actual->content_size_get());
    if (!crypto_rsa->push(padded_params.digest_padded(digest_padded).
                                        sig_actual(sig_actual))) {
        DOLE_FUNC_ERR("Failed crypto_rsa sig post_push");
        goto error;
    }

    /*
     * crypto_rsa->push() is responsible for freeing digest_padded
     * on completion.
     */
    return rsa_len;

error:
    if (digest_padded) {
        delete digest_padded;
    }
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
             n(n).
             d_e(d_e).
             user_ctx(callback_data);

    DOLE_FUNC_DEBUG("Setting up RSA ex_data");
    if (RSA_set_ex_data(rsa, rsa_ex_data_index, (void *)ex_data) <= 0) {
        DOLE_FUNC_ERR("Failed to set RSA ex data");
        goto error;
    }

    /*
     * Establish n/d/e in bignum format as required for RSA structure
     */
    bn_n = BN_bin2bn(n->read(), n->content_size_get(), NULL);
    if (!bn_n) {
        DOLE_FUNC_ERR("Failed BN_bin2bn for n");
        goto error;
    }

    bn_d_e = BN_bin2bn(d_e->read(), d_e->content_size_get(), NULL);
    if (!bn_d_e || BN_is_zero(bn_d_e)) {

        // use the most common default value
        bn_d_e = BN_new();
        if (bn_d_e) {
            BN_set_word(bn_d_e, RSA_F4);
        }
    }
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
local_rsa_ex_data_get(RSA *rsa)
{
    return static_cast<rsa_ex_data_t *>(RSA_get_ex_data(rsa, rsa_ex_data_index));
}

} // namespace dole
