#include "e_pse.h"
#include "pse_rsa.h"
#include <openssl/rsa.h>

static RSA_METHOD *pse_rsa_method = NULL;

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
    if(res ==  0)  {
        WARN("Failed to set RSA methods");
        return NULL;
    }
    return pse_rsa_method;
}

void pse_free_RSA_methods(void) 
{
    if(NULL != pse_rsa_method) {
        RSA_meth_free(pse_rsa_method);
        pse_rsa_method = NULL;
    }    
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
                                    to);
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

int pse_rsa_priv_enc(int flen, const unsigned char *from,
                     unsigned char *to, RSA *rsa, int padding)
{
    INFO("Inside");
    return 1;
}

int pse_rsa_priv_dec(int flen, const unsigned char *from,
                     unsigned char *to, RSA *rsa, int padding)
{
    INFO("Inside");
    return 1;
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

