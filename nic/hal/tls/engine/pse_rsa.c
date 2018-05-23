#include "e_pse.h"
#include "pse_rsa.h"
#include <openssl/rsa.h>

static RSA_METHOD *pse_rsa_method = NULL;
static int pse_rsa_ex_data_index = -1;

// Methods
static int pse_rsa_pub_enc(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding);
static int pse_rsa_pub_dec(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding);
static int pse_rsa_priv_enc(int flen, const unsigned char *from,
                            unsigned char *to, RSA *rsa, int padding);
static int pse_rsa_priv_dec(int flen, const unsigned char *from,
                            unsigned char *to, RSA *rsa, int padding);
static int pse_rsa_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx);
static int pse_rsa_bn_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                              const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
static int pse_rsa_free(RSA *rsa);

RSA_METHOD *pse_get_RSA_methods(void) 
{
    int res = 1;
    if(pse_rsa_method != NULL) {
        return pse_rsa_method; 
    }

    pse_rsa_method = RSA_meth_new("Pensando RSA offload", 0);
    if(pse_rsa_method == NULL) {
        WARN("Failed to malloc RSA_METHOD");
        return NULL;
    }

    res &= RSA_meth_set_pub_enc(pse_rsa_method, pse_rsa_pub_enc);
    res &= RSA_meth_set_pub_dec(pse_rsa_method, pse_rsa_pub_dec);
    res &= RSA_meth_set_priv_enc(pse_rsa_method, pse_rsa_priv_enc);
    res &= RSA_meth_set_priv_dec(pse_rsa_method, pse_rsa_priv_dec);
    res &= RSA_meth_set_mod_exp(pse_rsa_method, pse_rsa_mod_exp);
    res &= RSA_meth_set_bn_mod_exp(pse_rsa_method, pse_rsa_bn_mod_exp);
    res &= RSA_meth_set_finish(pse_rsa_method, pse_rsa_free);
    if(res ==  0)  {
        WARN("Failed to set RSA methods");
        return NULL;
    }
    if(pse_rsa_ex_data_index == -1) {
        pse_rsa_ex_data_index = RSA_get_ex_new_index(0,
                                                     "Pen SSL Hw Key Index",
                                                     NULL, NULL, NULL);
        INFO("Received RSA ex data index: %d", pse_rsa_ex_data_index);
    }
    return pse_rsa_method;
}

void pse_free_RSA_methods(void) 
{
    if(NULL != pse_rsa_method) {
        RSA_meth_free(pse_rsa_method);
        pse_rsa_method = NULL;
        if(pse_rsa_ex_data_index != -1) {
            CRYPTO_free_ex_index(CRYPTO_EX_INDEX_RSA, pse_rsa_ex_data_index);
            pse_rsa_ex_data_index = -1;
        }
    }    
}

PSE_RSA_EX_DATA* pse_rsa_get_ex_data(RSA *rsa)
{
    return (PSE_RSA_EX_DATA *)RSA_get_ex_data(rsa, pse_rsa_ex_data_index);    
}

static int 
pse_rsa_set_ex_data(RSA *rsa, uint32_t sig_gen_key_id, uint32_t decrypt_key_id)
{
    PSE_RSA_EX_DATA *ex_data = PSE_MALLOC(sizeof(PSE_RSA_EX_DATA));
    if(!ex_data) {
        WARN("Failed to allocate rsa ex data");
        return -1;
    }

    ex_data->sig_gen_key_id = sig_gen_key_id;
    ex_data->decrypt_key_id = decrypt_key_id;
    RSA_set_ex_data(rsa, pse_rsa_ex_data_index, ex_data);
    return 1;
}

static int pse_rsa_free(RSA *rsa)
{
    // Free ex data 
    PSE_RSA_EX_DATA *ex_data = pse_rsa_get_ex_data(rsa);
    if(ex_data) {
        PSE_FREE(ex_data);
        RSA_set_ex_data(rsa, pse_rsa_ex_data_index, NULL);
    }
    return 1;
}

int pse_rsa_pub_enc(int flen, const unsigned char *from,
                    unsigned char *to, RSA *rsa, int padding)
{
    int ret = 0, rsa_len = 0;
    unsigned char* buf = NULL;
    pse_buffer_t bn, be;
    const BIGNUM *n = NULL, *e = NULL, *d = NULL;
    BN_CTX *ctx = NULL;

    if(!from || !to || !rsa) {
        WARN("Invalid args");
        goto cleanup;
    }
    
    // Add padding 
    rsa_len = RSA_size(rsa);
    buf = PSE_MALLOC(rsa_len);
    if(!buf) {
        WARN("Failed to allocate input buffer");
        goto cleanup;
    }
    INFO("flen %d, rsa_len = %d, padding %d", flen, rsa_len, padding);

    switch (padding) {
    case RSA_PKCS1_PADDING:
        ret = RSA_padding_add_PKCS1_type_2(buf, rsa_len, from, flen);
        break;
    case RSA_PKCS1_OAEP_PADDING:
        ret = RSA_padding_add_PKCS1_OAEP(buf, rsa_len, from, flen, NULL, 0);
        break;
    case RSA_SSLV23_PADDING:
        ret = RSA_padding_add_SSLv23(buf, rsa_len, from, flen);
        break;
    case RSA_NO_PADDING:
        ret = RSA_padding_add_none(buf, rsa_len, from, flen);
        break;
    default:
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        goto cleanup;
    }
    if(ret <= 0) {
        WARN("Failed to add padding");
        goto cleanup;
    }

    INFO("buf:");
    HEX_DUMP(buf, rsa_len);

    // Get n and e
    if((ctx = BN_CTX_new()) == NULL) {
        WARN("Failed to allocate BN ctx");
        goto cleanup;
    }

    BN_CTX_start(ctx);
    n = BN_CTX_get(ctx);
    e = BN_CTX_get(ctx);
    d = BN_CTX_get(ctx);

    RSA_get0_key(rsa, &n, &e, &d);
    pse_BN_to_buffer_pad(n, &bn, rsa_len);
    pse_BN_to_buffer_pad(e, &be, rsa_len);

    LOG_BUFFER("bn", bn);
    LOG_BUFFER("be", be);
 
#ifndef NO_PEN_HW_OFFLOAD
    ret = pd_tls_asym_rsa2k_encrypt(bn.data,
                                    be.data,
                                    buf,
                                    to,
                                    true,
                                    (const uint8_t *)engine_pse_id);
#else 
    ret = RSA_meth_get_pub_enc(RSA_PKCS1_OpenSSL())(flen, from, to, rsa, padding);
#endif

    INFO("to:");
    HEX_DUMP(to, rsa_len);
    ret = rsa_len;

cleanup:
    if(ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    pse_free_buffer(&bn);
    pse_free_buffer(&be);
    return ret;

}

int pse_rsa_pub_dec(int flen, const unsigned char *from,
                    unsigned char *to, RSA *rsa, int padding)
{
    INFO("Inside");
    return 1;
    
}

/*
 * Low level implementation of RSA sign
 */
int pse_rsa_priv_enc(int flen, const unsigned char *from,
                     unsigned char *to, RSA *rsa, int padding)
{
    INFO("Inside");
    int ret = 0, rsa_len = 0;
    unsigned char* buf = NULL;
    pse_buffer_t bn;
    const BIGNUM *n = NULL, *e = NULL, *d = NULL;
    BN_CTX *ctx = NULL;

    if(!from || !to || !rsa) {
        WARN("Invalid args");
        goto cleanup;
    }
    // Get Hw Key index
    PSE_RSA_EX_DATA *ex_data = pse_rsa_get_ex_data(rsa);
    if(!ex_data) {
        WARN("Failed to get rsa ex data");
        goto cleanup;
    }

    rsa_len = RSA_size(rsa);
    buf = PSE_MALLOC(rsa_len);
    if(!buf) {
        WARN("Failed to allocate input buffer");
        goto cleanup;
    }
    INFO("flen %d, rsa_len = %d, padding %d, key_idx: %d", 
                    flen, rsa_len, padding, ex_data->sig_gen_key_id);

    switch (padding) {
    case RSA_PKCS1_PADDING:
        ret = RSA_padding_add_PKCS1_type_1(buf, rsa_len, from, flen);
        break;
    case RSA_X931_PADDING:
        ret = RSA_padding_add_X931(buf, rsa_len, from, flen);
        break;
    case RSA_NO_PADDING:
        ret = RSA_padding_add_none(buf, rsa_len, from, flen);
        break;
    case RSA_SSLV23_PADDING:
    default:
        RSAerr(RSA_F_RSA_OSSL_PUBLIC_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        goto cleanup;
    }
    if(ret <= 0) {
        WARN("Failed to add padding");
        goto cleanup;
    }

    INFO("buf:");
    HEX_DUMP(buf, rsa_len);

    // Get n and d
    if ((ctx = BN_CTX_new()) == NULL) {
        WARN("Failed to allocate BN ctx");
        goto cleanup;
    }
    
    BN_CTX_start(ctx);
    n = BN_CTX_get(ctx);
    e = BN_CTX_get(ctx);
    d = BN_CTX_get(ctx);
     
    RSA_get0_key(rsa, &n, &e, &d);
    pse_BN_to_buffer_pad(n, &bn, rsa_len);
    LOG_BUFFER("bn", bn);
 
#ifndef NO_PEN_HW_OFFLOAD
    ret = pd_tls_asym_rsa2k_sig_gen(ex_data->sig_gen_key_id,
                                    bn.data,
                                    NULL,
                                    buf,
                                    to);
#else 
    ret = RSA_meth_get_priv_enc(RSA_PKCS1_OpenSSL())(flen, from, to, rsa, padding);
#endif

    INFO("to:");
    HEX_DUMP(to, rsa_len);
    ret = rsa_len;

cleanup:
    if(ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    pse_free_buffer(&bn);
    return ret;
}

int pse_rsa_priv_dec(int flen, const unsigned char *from,
                     unsigned char *to, RSA *rsa, int padding)
{
    int                 ret = -1;
    int                 rsa_len = 0;
    PSE_RSA_EX_DATA     *ex_data = NULL;
    INFO("Inside");

    if(!from || !to || !rsa) {
        WARN("Invalid args");
        goto cleanup;
    }
    
    ex_data = pse_rsa_get_ex_data(rsa);
    if(!ex_data) {
        WARN("Failed to get rsa ex data");
        goto cleanup;
    }

    rsa_len = RSA_size(rsa);
    
    INFO("flen %d, rsa_len: %d, padding: %d, key_id: %d",
         flen, rsa_len, padding, ex_data->decrypt_key_id);
    if(flen > rsa_len) {
        INFO("ERROR: flen is greater than rsa_len");
        goto cleanup;
    }

#ifndef NO_PEN_HW_OFFLOAD
    ret = pd_tls_asym_rsa2k_crt_decrypt(ex_data->decrypt_key_id,
                                        NULL, NULL, NULL, NULL, NULL,
                                        (uint8_t *)from, to);
    INFO("ret: %d", ret);
    if(ret < 0) {
        WARN("rsa decrypt failed...");
        goto cleanup;
    }

    return rsa_len;
#endif

cleanup:
    return ret;
}

int 
pse_rsa_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx)
{
    INFO("Inside");
    return RSA_meth_get_mod_exp(RSA_PKCS1_OpenSSL())(r0, I, rsa, ctx);
}

int pse_rsa_bn_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                       const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
    INFO("Inside");
    return RSA_meth_get_bn_mod_exp(RSA_PKCS1_OpenSSL())(r, a, p, m, ctx, m_ctx);
}


static RSA* pse_get_rsa(PSE_KEY* key) 
{
    BIGNUM *rsa_n = NULL, *rsa_e = NULL;
    RSA *rsa = NULL;
    PSE_RSA_KEY *rsa_key = NULL;

    if(!key) {
        WARN("Invalid args");
        goto err;
    }
    
    rsa_key = &(key->u.rsa_key);
    INFO("n: %d", rsa_key->rsa_n.len);
    rsa_n = pse_buffer_to_BN(&(rsa_key->rsa_n));
    if(!rsa_n) {
        WARN("Failed to get the value of n");
        goto err;
    }
    
    rsa_e = pse_buffer_to_BN(&(rsa_key->rsa_e));
    if(!rsa_e || BN_is_zero(rsa_e)) {
        // use the most common default value
        rsa_e = BN_new();
        if(!rsa_e) {
            WARN("Failed to allocate rsa_e");
            goto err;
        }
        BN_set_word(rsa_e, RSA_F4);
    }

    rsa = RSA_new();
    if(rsa == NULL) {
        WARN("Failed to allocate RSA");
        goto err;
    }
    
    RSA_set0_key(rsa, rsa_n, rsa_e, NULL);
    
    RSA_set_method(rsa, pse_get_RSA_methods()); 

    return rsa;

err:
    if(rsa_n)
        BN_clear_free(rsa_n);
    if(rsa_e)
        BN_clear_free(rsa_e);
    return NULL;
}

/* 
 * BUILD EVP PKey
 */
EVP_PKEY* 
pse_rsa_get_evp_key(ENGINE* engine, PSE_KEY* key,
                    UI_METHOD *ui_method, void *callback_data)
{
    EVP_PKEY    *pkey = NULL;
    RSA         *rsa = NULL;

    INFO("Inside");    
    
    rsa = pse_get_rsa(key);
    if(!rsa) {
        return NULL;    
    }

    pkey = EVP_PKEY_new();
    if(!pkey) {
        WARN("Failed to allocate pkey");
        goto err;
    }
    
    EVP_PKEY_set1_RSA(pkey, rsa);
    pse_rsa_set_ex_data(rsa,
                        key->u.rsa_key.sign_key_id,
                        key->u.rsa_key.decrypt_key_id);

    return pkey;

err:
    if(rsa)
        RSA_free(rsa);
    return NULL;
}


