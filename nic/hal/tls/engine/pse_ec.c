#include "e_pse.h"
#include "pse_ec.h"
static EC_KEY_METHOD *pse_ec_method = NULL;
static int pse_ec_ex_data_index = -1;

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
static void pse_ec_key_free(EC_KEY *eckey);


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
    EC_KEY_METHOD_set_init(pse_ec_method,      // meth
                           NULL,               // init
                           pse_ec_key_free,    // finish
                           NULL,               // copy
                           NULL,               // set_group
                           NULL,               // set_private
                           NULL                // set_public
                           );
    EC_KEY_METHOD_set_sign(pse_ec_method,
                           pse_ecdsa_sign,
                           NULL,
                           pse_ecdsa_sign_sig);
    EC_KEY_METHOD_set_verify(pse_ec_method,
                             pse_ecdsa_verify,
                             pse_ecdsa_verify_sig);

    if(pse_ec_ex_data_index == -1) {
        pse_ec_ex_data_index = EC_KEY_get_ex_new_index(0, 
                                                       "Pen SSL Hw external data",
                                                        NULL, NULL, NULL);
        INFO("Received EC ex data index: %d", pse_ec_ex_data_index);
    }
    return pse_ec_method;
}

void pse_free_EC_methods(void) 
{
    if(NULL != pse_ec_method) {
        EC_KEY_METHOD_free(pse_ec_method);
        pse_ec_method = NULL;
    }
    if(pse_ec_ex_data_index != -1) {
        CRYPTO_free_ex_index(CRYPTO_EX_INDEX_EC_KEY, pse_ec_ex_data_index);
        pse_ec_ex_data_index = -1;
    }
}

PSE_EC_EX_DATA*
pse_ec_get_ex_data(EC_KEY *eckey)
{
    return (PSE_EC_EX_DATA*) EC_KEY_get_ex_data(eckey, pse_ec_ex_data_index);
}

static int
pse_ec_set_ex_data(EC_KEY *eckey, uint32_t hw_key_index)
{
    PSE_EC_EX_DATA *ex_data = PSE_MALLOC(sizeof(PSE_EC_EX_DATA));
    if(!ex_data) {
        WARN("Failed to allocate eckey ex data");
        return -1;
    }
    ex_data->hw_key_index = hw_key_index;
    EC_KEY_set_ex_data(eckey, pse_ec_ex_data_index, (void *)ex_data);
    return 1;
}

static void
pse_ec_key_free(EC_KEY *eckey)
{
    PSE_EC_EX_DATA *ex_data = pse_ec_get_ex_data(eckey);
    if(ex_data) {
        PSE_FREE(ex_data);
        EC_KEY_set_ex_data(eckey, pse_ec_ex_data_index, NULL);
    }
}

int pse_ecdsa_sign(int type, const unsigned char *dgst,
                   int dlen, unsigned char *sig,
                   unsigned int *siglen,
                   const BIGNUM *kinv, const BIGNUM *r,
                   EC_KEY *eckey)
{
    INFO("Inside");
    ECDSA_SIG *s;
    RAND_seed(dgst, dlen);
    s = pse_ecdsa_sign_sig(dgst, dlen, kinv, r, eckey);
    if (s == NULL) {
        *siglen = 0;
        return 0;
    }
    *siglen = i2d_ECDSA_SIG(s, &sig);
    ECDSA_SIG_free(s);
    return 1;
}

ECDSA_SIG *pse_ecdsa_sign_sig(const unsigned char *dgst,
                              int dgst_len,
                              const BIGNUM *in_kinv,
                              const BIGNUM *in_r,
                              EC_KEY *eckey)
{
    int             ret = 0;
    BN_CTX          *ctx = NULL;
    int             i = 0, siglen = 0;
    const BIGNUM    *order = NULL;
    const EC_GROUP  *group = NULL;;
    ECDSA_SIG       *sig = NULL;
    BIGNUM          *k = NULL, *m = NULL;
    pse_buffer_t    bk, bh;
    uint8_t         buf_s[256] = {0}, buf_r[256] = {0};
    BIGNUM          *r = NULL, *s = NULL;
    const PSE_EC_EX_DATA  *ex_data = NULL;
    INFO("Inside");

#ifdef NO_PEN_HW_OFFLOAD
    ECDSA_SIG* (*osl_sign_sig) (const unsigned char *dgst,
                                int dlen,
                                const BIGNUM *kinv, const BIGNUM *r,
                                EC_KEY *eckey) = NULL;
    EC_KEY_METHOD_get_sign((EC_KEY_METHOD*)EC_KEY_OpenSSL(), NULL, NULL, &osl_sign_sig);
    return (*osl_sign_sig)(dgst, dgst_len, in_kinv, in_r, eckey);
#endif

    ex_data = pse_ec_get_ex_data(eckey);
    if(!ex_data) {
        WARN("Failed to find the exdata from the key");
        return NULL;
    }

    group = EC_KEY_get0_group(eckey);
    if(group == NULL) {
        WARN("Failed to get group, priv key");
        return NULL;
    }
    
    order = EC_GROUP_get0_order(group);
    if (order == NULL) {
        WARN("Failed to get order");
        return NULL;
    }
    i = BN_num_bits(order);

    if((ctx = BN_CTX_new()) == NULL) {
        WARN("Failed to allocate BN ctx");
        goto cleanup;
    }
    
    BN_CTX_start(ctx);
    k = BN_CTX_get(ctx);
    m = BN_CTX_get(ctx);
    if(!k || !m) {
        WARN("Failed to allocate memory for m, k");
        goto cleanup;
    }
    /*
     * Need to truncate digest if it is too long: first truncate whole bytes.
     */

    if (8 * dgst_len > i)
        dgst_len = (i + 7) / 8;

    if (!BN_bin2bn(dgst, dgst_len, m)) {
        WARN("Failed to covert dgst to m");
        return NULL;
    }

    /* If still too long truncate remaining bits with a shift */
    if ((8 * dgst_len > i) && !BN_rshift(m, m, 8 - (i & 0x7))) {
        WARN("Failed to truncate dgst")
        goto cleanup;
    }

    int iter = 0;
    do {
        // We always generate random number. This will be moved to lower layers. 
        // allocate k
        do  {
            if(!BN_rand_range(k, order)) {
                WARN("Failed to allocate the random number k");
                goto cleanup;
            }
        } while (BN_is_zero(k));

        pse_BN_to_buffer(k, &bk);
        pse_BN_to_buffer(m, &bh);

        INFO("Sign using key_idx: %d, k len: %d, h len: %d, dgst_len: %d, i: %d",
             ex_data->hw_key_index, bk.len, bh.len, dgst_len, i);

#ifndef NO_PEN_HW_OFFLOAD
        ret = pd_tls_asym_ecdsa_p256_sig_gen(ex_data->hw_key_index,
                                             NULL,              // p
                                             NULL,              // n
                                             NULL, NULL,        // xg, yg
                                             NULL, NULL, NULL,  // a, b, da
                                             bk.data,           // k
                                             bh.data,           // h,
                                             buf_r,             // r
                                             buf_s);             // s
        INFO("ret : %d", ret);
        if(ret < 0) {
            WARN("ecdsa_sign_gen failed...retrying..");
            continue;
        }
        break;
    } while (++iter < 10);

    if(ret < 0) {
        WARN("signature generation failed..");
        goto cleanup;
    }
#endif
    siglen = i / 8;
    r = BN_bin2bn(buf_r, siglen, NULL);
    s = BN_bin2bn(buf_s, siglen, NULL);

    sig = ECDSA_SIG_new();
    if(sig == NULL) {
        WARN("Failed to allocate sig");
        goto cleanup;
    }

    ECDSA_SIG_set0(sig, r, s);
 cleanup:
    if(ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    return sig;
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
        WARN("Failure: ECDSA_SIG length mismatch: derlen %d, sig_len: %d",
             derlen, sig_len);
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
    int                 ret = -1;
    BN_CTX              *ctx = NULL;
    BIGNUM              *order = NULL;
    const EC_GROUP      *group = NULL;
    const EC_POINT      *pub_key;
    BIGNUM              *p = NULL, *a = NULL, *b = NULL;
    BIGNUM              *xg = NULL, *yg = NULL;
    BIGNUM              *xp = NULL, *yp = NULL;
    const EC_POINT      *ec_point = NULL;
    const BIGNUM        *sig_r = NULL, *sig_s = NULL;
    int                 siglen = 0;
    pse_buffer_t        bp, bn, bxg,byg;
    pse_buffer_t        ba, bb, bxq,byq;
    pse_buffer_t        br, bs;

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
    
    siglen = BN_num_bytes(order);

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
    INFO("Siglen: %d", siglen);

    pse_BN_to_buffer_pad(p, &bp, siglen);
    pse_BN_to_buffer_pad(order, &bn, siglen);
    pse_BN_to_buffer_pad(xg, &bxg, siglen);
    pse_BN_to_buffer_pad(yg, &byg, siglen);
    pse_BN_to_buffer_pad(a, &ba, siglen);
    pse_BN_to_buffer_pad(b, &bb, siglen);
    pse_BN_to_buffer_pad(xp, &bxq, siglen);
    pse_BN_to_buffer_pad(yp, &byq, siglen);
    pse_BN_to_buffer_pad(sig_r, &br, siglen);
    pse_BN_to_buffer_pad(sig_s, &bs, siglen);

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
#else
    ret = 1;
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

static EC_KEY *
pse_ec_get_ec_key(const PSE_KEY *key)
{
    EC_KEY *eckey = NULL;

    if(!key)
        return NULL;

    eckey = EC_KEY_new();
    if(!eckey) {
        WARN("Failed to allocate eckey");
        return NULL;
    }

    if(key->u.ec_key.group) {       
        if(EC_KEY_set_group(eckey, key->u.ec_key.group) < 0) {
            WARN("Failed to set the group");
            goto err;
        }
    }

    if(key->u.ec_key.point) {
        if(EC_KEY_set_public_key(eckey, key->u.ec_key.point) < 0) {
            WARN("Failed to set the point");
            goto err;
        }
    }

    return eckey;
err:
    if(eckey) {
        EC_KEY_free(eckey);
    }
    return NULL;
}

EVP_PKEY*
pse_ec_get_evp_key(ENGINE* engine, PSE_KEY *key,
                             UI_METHOD *ui_method, void *callback_data)
{
    EVP_PKEY    *pkey = NULL;
    EC_KEY      *eckey = NULL;

    INFO("Inside");

    eckey = pse_ec_get_ec_key(key);
    if(!eckey) {
        WARN("Failed to get eckey");
        return NULL;
    }

    pkey = EVP_PKEY_new();
    if(!pkey) {
        WARN("Failed to allocate pkey");
        goto err;
    }

    EVP_PKEY_set1_EC_KEY(pkey, eckey);
    pse_ec_set_ex_data(eckey, key->index);

    return pkey;

err:
    if(eckey) {
        EC_KEY_free(eckey);
    }
    return NULL;
}
