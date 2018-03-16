#include "e_pse.h"
#include "pse_ec.h"
static EC_KEY_METHOD *pse_ec_method = NULL;

// Methods
static int pse_ecdsa_sign(int type, const unsigned char *dgst,
                          int dlen, unsigned char *sig,
                          unsigned int *siglen,
                          const BIGNUM *kinv, const BIGNUM *r,
                          EC_KEY *eckey);
static ECDSA_SIG *pse_ecdsa_sign_sig(const unsigned char *dgst,
                                     int dgst_len,
                                     const BIGNUM *in_kinv,
                                     const BIGNUM *in_r,
                                     EC_KEY *eckey);
static int pse_ecdsa_verify(int type, const unsigned char *dgst,
                            int dgst_len, const unsigned char *sigbuf,
                            int sig_len, EC_KEY *eckey);
static int pse_ecdsa_verify_sig(const unsigned char *dgst,
                                int dgst_len, const ECDSA_SIG *sig,
                                EC_KEY *eckey);


EC_KEY_METHOD *pse_get_EC_methods(void) 
{
    if(pse_ec_method != NULL) {
        return pse_ec_method;   
    }

    pse_ec_method = EC_KEY_METHOD_new(pse_ec_method);
    if(pse_ec_method == NULL) {
        WARN("Failed to malloc EC_KEY_METHOD");
        return NULL;
    }
    EC_KEY_METHOD_set_sign(pse_ec_method,
                           pse_ecdsa_sign,
                           NULL,
                           pse_ecdsa_sign_sig);
    EC_KEY_METHOD_set_verify(pse_ec_method,
                             pse_ecdsa_verify,
                             pse_ecdsa_verify_sig);

    return pse_ec_method;
}

void pse_free_EC_methods(void) 
{
    if(NULL != pse_ec_method) {
        EC_KEY_METHOD_free(pse_ec_method);
        pse_ec_method = NULL;
    }    
}

int pse_ecdsa_sign(int type, const unsigned char *dgst,
                   int dlen, unsigned char *sig,
                   unsigned int *siglen,
                   const BIGNUM *kinv, const BIGNUM *r,
                   EC_KEY *eckey)
{
    INFO("Inside");
    return 1;
}
ECDSA_SIG *pse_ecdsa_sign_sig(const unsigned char *dgst,
                              int dgst_len,
                              const BIGNUM *in_kinv,
                              const BIGNUM *in_r,
                              EC_KEY *eckey)
{
    INFO("Inside");
    return NULL;
}

int pse_ecdsa_verify(int type, const unsigned char *dgst,
                     int dgst_len, const unsigned char *sigbuf,
                     int sig_len, EC_KEY *eckey)
{
    int ret = -1;
    ECDSA_SIG *dsig;
    unsigned char *der = NULL;
    int derlen = -1;

    INFO("Inside");
    dsig = ECDSA_SIG_new();
    if(dsig == NULL) {
        WARN("Failed to allocate ECDSA_SIG dsig");
        return ret;
    }

    if(d2i_ECDSA_SIG(&dsig, &sigbuf, sig_len) == NULL) {
        WARN("Failed to convert sig_buf to decoded sig");
        return ret;
    }

    /* Ensure signature uses DER and doesn't have trailing garbage */
    derlen = i2d_ECDSA_SIG(dsig, &der);
    if(derlen != sig_len) {
        WARN("Failure: ECDSA_SIG length mismatch: derlen %d, sig_len: %d",            derlen, sig_len);
        goto cleanup;
    }

    ret = pse_ecdsa_verify_sig(dgst, dgst_len, dsig, eckey);

cleanup:
    OPENSSL_clear_free(der, derlen);
    ECDSA_SIG_free(dsig);
    return ret;
}

int pse_ecdsa_verify_sig(const unsigned char *dgst,
                         int dgst_len, const ECDSA_SIG *sig,
                         EC_KEY *eckey)
{
    INFO("Inside");
    int ret = -1;
    BN_CTX *ctx = NULL;
    BIGNUM *order = NULL;
    const EC_GROUP *group = NULL;
    const EC_POINT *pub_key;
    BIGNUM *p = NULL, *a = NULL, *b = NULL;
    BIGNUM *xg = NULL, *yg = NULL, *xp = NULL, *yp = NULL;
    const EC_POINT *ec_point;
    const BIGNUM *sig_r = NULL, *sig_s = NULL;

    pse_buffer_t bp, bn, bxg,byg;
    pse_buffer_t ba, bb, bxq,byq;
    pse_buffer_t br, bs;

    if((eckey == NULL) ||
       ((group = EC_KEY_get0_group(eckey)) == NULL) ||
       ((pub_key = EC_KEY_get0_public_key(eckey)) == NULL) ||
       (sig == NULL)) {
        WARN("Invalid arguments");
        return ret;
    }
    
    if ((ec_point = EC_GROUP_get0_generator(group)) == NULL) {
        WARN("Failed to retrieve ec_point");
        return ret;
    }

    if((ctx = BN_CTX_new()) == NULL) {
        WARN("Failed to allocate BN ctx");
        goto cleanup;
    }
    
    BN_CTX_start(ctx);
    p = BN_CTX_get(ctx);
    a = BN_CTX_get(ctx);
    b = BN_CTX_get(ctx);
    xg = BN_CTX_get(ctx);
    yg = BN_CTX_get(ctx);
    xp = BN_CTX_get(ctx);
    yp = BN_CTX_get(ctx);
    order = BN_CTX_get(ctx);
    if(order == NULL) {
        WARN("Failed to allocate parameters");
        goto cleanup;
    }

    if(!EC_GROUP_get_order(group, order, ctx)) {
        WARN("Failed to get order from the group");
        goto cleanup;
    }

    ECDSA_SIG_get0(sig, &sig_r, &sig_s);
    if(BN_is_zero(sig_r) || BN_is_negative(sig_r) || BN_ucmp(sig_r, order) >= 0 ||
       BN_is_zero(sig_s) || BN_is_negative(sig_s) || BN_ucmp(sig_s, order) >= 0) {
        WARN("sig_r and sig_s validation failed");
        ret = 0;
        goto cleanup;
    }
    
    if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) ==
            NID_X9_62_prime_field) {
        if(!EC_GROUP_get_curve_GFp(group, p, a, b, ctx)) {
            WARN("Failed to get GFp parameters");
            goto cleanup;
        } 
        if(!EC_POINT_get_affine_coordinates_GFp(group, ec_point,
                                                xg, yg, ctx)) {
            WARN("Failed to get ec_point GFp parameters");
            goto cleanup;
        }
        if(!EC_POINT_get_affine_coordinates_GFp(group, pub_key,
                                                xp, yp, ctx)) {
            WARN("Failed to get pub_key GFp parameters");
            goto cleanup;
        }
    } else { 
        if(!EC_GROUP_get_curve_GF2m(group, p, a, b, ctx)) {
            WARN("Failed to get GF2m parameters");
            goto cleanup;
        } 
        if(!EC_POINT_get_affine_coordinates_GF2m(group, ec_point,
                                                 xg, yg, ctx)) {
            WARN("Failed to get ec_point GF2m parameters");
            goto cleanup;
        }
        if(!EC_POINT_get_affine_coordinates_GF2m(group, pub_key,
                                                 xp, yp, ctx)) {
            WARN("Failed to get pub_key GF2m parameters");
            goto cleanup;
        } 
    }

    pse_BN_to_buffer(p, &bp);
    pse_BN_to_buffer(order, &bn);
    pse_BN_to_buffer(xg, &bxg);
    pse_BN_to_buffer(yg, &byg);
    pse_BN_to_buffer(a, &ba);
    pse_BN_to_buffer(b, &bb);
    pse_BN_to_buffer(xp, &bxq);
    pse_BN_to_buffer(yp, &byq);
    pse_BN_to_buffer(sig_r, &br);
    pse_BN_to_buffer(sig_s, &bs);

    //LOG_BUFFER("p:", bp); 
    INFO("ECDSA parameters: ");
    LOG_BUFFER("bp", bp);
    LOG_BUFFER("bn", bn);
    LOG_BUFFER("bxg", bxg);
    LOG_BUFFER("byg", byg);
    LOG_BUFFER("ba", ba);
    LOG_BUFFER("bb", bb);
    LOG_BUFFER("bxq", bxq);
    LOG_BUFFER("byq", byq);
    LOG_BUFFER("br", br);
    LOG_BUFFER("bs", bs);

#ifndef NO_PEN_HW_OFFLOAD
    // Send verify request to barco
    ret = pd_tls_asym_ecdsa_p256_sig_verify(bp.data,
                                            bn.data, 
                                            bxg.data,
                                            byg.data,
                                            ba.data,
                                            bb.data,
                                            bxq.data,
                                            byq.data,
                                            br.data,
                                            bs.data,
                                            (uint8_t*)dgst);
#endif
    INFO("Return value: ret %d", ret);
cleanup:
    if(ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    pse_free_buffer(&bp);
    pse_free_buffer(&bn);
    pse_free_buffer(&bxg);
    pse_free_buffer(&byg);
    pse_free_buffer(&ba);
    pse_free_buffer(&bb);
    pse_free_buffer(&bxq);
    pse_free_buffer(&byq);
    pse_free_buffer(&br);
    pse_free_buffer(&bs);
    return ret;
}
