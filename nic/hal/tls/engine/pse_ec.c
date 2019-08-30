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

typedef int (*PFUNC_GEN_KEY)(EC_KEY *);
typedef int (*PFUNC_COMP_KEY)(unsigned char **,
        size_t *,
        const EC_POINT *,
        const EC_KEY *);

EC_KEY_METHOD *pse_get_EC_methods(void) 
{
    EC_KEY_METHOD *def_ec_meth = (EC_KEY_METHOD *)EC_KEY_get_default_method();
    PFUNC_GEN_KEY gen_key_pfunc = NULL;
    PFUNC_COMP_KEY comp_key_pfunc = NULL;


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

    EC_KEY_METHOD_get_keygen(def_ec_meth, &gen_key_pfunc);
    EC_KEY_METHOD_set_keygen(pse_ec_method, gen_key_pfunc);

    EC_KEY_METHOD_get_compute_key(def_ec_meth, &comp_key_pfunc);
    EC_KEY_METHOD_set_compute_key(pse_ec_method, comp_key_pfunc);

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
pse_ec_set_ex_data(EC_KEY *eckey, PSE_KEY* key, void *caller_ctx)
{
    PSE_EC_EX_DATA *ex_data = PSE_CALLOC(1, sizeof(PSE_EC_EX_DATA));
    if(!ex_data) {
        WARN("Failed to allocate eckey ex data");
        return -1;
    }
    ex_data->hw_key_index = key->u.ec_key.key_id;
    ex_data->offload = key->u.ec_key.offload;
    ex_data->caller_ctx = caller_ctx;
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
    const PSE_EC_EX_DATA  *ex_data;

    *siglen = 0;
    ex_data = pse_ec_get_ex_data(eckey);
    if(!ex_data) {
        WARN("Failed to find the exdata from the key");
        return 0;
    }

    s = pse_ecdsa_sign_sig(dgst, dlen, kinv, r, eckey);
    if (!ex_data->offload.skip_DER) {
        if (s == NULL) {
            return 0;
        }
        *siglen = i2d_ECDSA_SIG(s, &sig);
    }
    ECDSA_SIG_free(s);
    return 1;
}

static int
pse_digest_truncate(const unsigned char *dgst,
                    int dgst_len,
                    uint32_t bits_len,
                    BIGNUM *m,
                    bool *dsgt_trunc)
{
    /*
     * Need to truncate digest if it is too long: first truncate whole bytes.
     */
    *dsgt_trunc = false;
    if ((8 * dgst_len) != bits_len) {
        *dsgt_trunc = true;
        if ((8 * dgst_len) > bits_len)
            dgst_len = (bits_len + 7) / 8;

        if (!BN_bin2bn(dgst, dgst_len, m)) {
            WARN("Failed to covert dgst to m");
            return -1;
        }

        /* If still too long truncate remaining bits with a shift */
        if ((8 * dgst_len > bits_len) && !BN_rshift(m, m, 8 - (bits_len & 0x7))) {
            WARN("Failed to truncate dgst")
            return -1;
        }
    }
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
    int             bits_len = 0, siglen = 0;
    int             ceil_bytes_len;
    const BIGNUM    *order = NULL;
    const EC_GROUP  *group = NULL;;
    ECDSA_SIG       *sig = NULL;
    BIGNUM          *k = NULL, *m = NULL;
    pse_buffer_t    bk, bh;
    uint8_t         buf_s[256] = {0}, buf_r[256] = {0};
    BIGNUM          *r = NULL, *s = NULL;
    const PSE_EC_EX_DATA  *ex_data = NULL;
    const PSE_EC_OFFLOAD_METHOD *offload_method;
    const PSE_OFFLOAD_MEM_METHOD *mem_method;
    const unsigned char *hash_input;
    PSE_EC_SIGN_PARAM param;
    bool dsgt_trunc = false;
    INFO("Inside");

    ex_data = pse_ec_get_ex_data(eckey);
    if(!ex_data) {
        WARN("Failed to find the exdata from the key");
        return NULL;
    }

    offload_method = ex_data->offload.offload_method;
    if (!offload_method) {
        WARN("Failed to get rsa offload_method");
        return NULL;
    }
    mem_method = offload_method->mem_method;

#ifdef NO_PEN_HW_OFFLOAD
    if (mem_method) {
        WARN("mem_method usage implies HW offload is required");
        return NULL;
    }
    ECDSA_SIG* (*osl_sign_sig) (const unsigned char *dgst,
                                int dlen,
                                const BIGNUM *kinv, const BIGNUM *r,
                                EC_KEY *eckey) = NULL;
    EC_KEY_METHOD_get_sign((EC_KEY_METHOD*)EC_KEY_OpenSSL(), NULL, NULL, &osl_sign_sig);
    return (*osl_sign_sig)(dgst, dgst_len, in_kinv, in_r, eckey);
#endif

    hash_input = dgst;
    if (mem_method) {
        hash_input = mem_method->read((PSE_OFFLOAD_MEM *)dgst);
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
    bits_len = BN_num_bits(order);
    ceil_bytes_len = (bits_len + 7) / 8;

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
     * Need to truncate digest if it is too long
     */
    if (pse_digest_truncate(hash_input, dgst_len, bits_len, m, &dsgt_trunc) <= 0) {
        goto cleanup;
    }

    pse_ec_sign_param_init(&param);
    param.key_idx = ex_data->hw_key_index;
    param.async = true;
    param.caller_unique_id = (const uint8_t *)engine_pse_id;
    param.wait_for_completion = ex_data->offload.wait_for_completion;

    int iter = 0;
    do {

        if (ex_data->offload.sign.k_random) {
            if (dsgt_trunc) {
                if (BN_bn2binpad(m, mem_method->read((PSE_OFFLOAD_MEM *)dgst),
                                 ceil_bytes_len) <= 0) {
                    WARN("Failed BN_bn2binpad");
                    goto cleanup;
                }
                mem_method->content_size_set((PSE_OFFLOAD_MEM *)dgst,
                                             ceil_bytes_len);
                mem_method->write_thru((PSE_OFFLOAD_MEM *)dgst);
            }
            param.k_random = (uint8_t *)ex_data->offload.sign.k_random;
            param.hash_input = (uint8_t *)dgst;
            param.sig_output_vec = (uint8_t *)ex_data->offload.sign.sig_output_vec;
            INFO("Sign using key_idx: %d, dgst_len: %d, bits_len: %d",
                 ex_data->hw_key_index, dgst_len, bits_len);

        } else {

            // We always generate random number. This will be moved to lower layers. 
            // allocate k
            RAND_seed(hash_input, dgst_len);
            param.r_output = buf_r;
            param.s_output = buf_s;

            do  {
                if(!BN_rand_range(k, order)) {
                    WARN("Failed to allocate the random number k");
                    goto cleanup;
                }
            } while (BN_is_zero(k));

            pse_BN_to_buffer_pad(k, &bk, (bits_len/8));
            pse_BN_to_buffer_pad(m, &bh, (bits_len/8));
            param.k_random = bk.data;
            param.hash_input = bh.data;

            INFO("Sign using key_idx: %d, k len: %d, h len: %d, dgst_len: %d, bits_len: %d",
                 ex_data->hw_key_index, bk.len, bh.len, dgst_len, bits_len);
        }

#ifndef NO_PEN_HW_OFFLOAD
        ret = offload_method->sign(ex_data->caller_ctx, &param);

        INFO("ret : %d", ret);
        if((ret < 0) && !ex_data->offload.sign.k_random) {
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

    if (!ex_data->offload.skip_DER) {
        siglen = bits_len / 8;
        r = BN_bin2bn(buf_r, siglen, NULL);
        s = BN_bin2bn(buf_s, siglen, NULL);

        sig = ECDSA_SIG_new();
        if(sig == NULL) {
            WARN("Failed to allocate sig");
            goto cleanup;
        }

        ECDSA_SIG_set0(sig, r, s);
    }
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
    ECDSA_SIG *dsig = NULL;
    unsigned char *der = NULL;
    int derlen = -1;
    const PSE_EC_EX_DATA  *ex_data;

    INFO("Inside");
    ex_data = pse_ec_get_ex_data(eckey);
    if(!ex_data) {
        WARN("Failed to find the exdata from the key");
        return 0;
    }

    if (!ex_data->offload.skip_DER) {
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

    } else {
        ret = pse_ecdsa_verify_sig(dgst, dgst_len, (ECDSA_SIG *)sigbuf, eckey);
    }


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
    BIGNUM              *m = NULL;
    int                 bits_len = 0, siglen = 0;
    int                 ceil_bytes_len;
    pse_buffer_t        bp, bn, bxg,byg;
    pse_buffer_t        ba, bb, bxq,byq;
    pse_buffer_t        br, bs;
    const PSE_EC_EX_DATA  *ex_data = NULL;
    const PSE_EC_OFFLOAD_METHOD *offload_method;
    const PSE_OFFLOAD_MEM_METHOD *mem_method;
    const unsigned char *hash_input;
    PSE_EC_VERIFY_PARAM param;
    bool dsgt_trunc = false;

    if((eckey == NULL) ||
       ((group = EC_KEY_get0_group(eckey)) == NULL) ||
       (sig == NULL)) {
        WARN("Invalid arguments");
        return ret;
    }
    
    ex_data = pse_ec_get_ex_data(eckey);
    if(!ex_data) {
        WARN("Failed to find the exdata from the key");
        return ret;
    }

    offload_method = ex_data->offload.offload_method;
    if (!offload_method) {
        WARN("Failed to get rsa offload_method");
        return ret;
    }
    mem_method = offload_method->mem_method;

    pse_ec_verify_param_init(&param);
    param.key_idx = ex_data->hw_key_index;
    param.hash_input = (uint8_t *)dgst;
    param.async = true;
    param.caller_unique_id = (const uint8_t *)engine_pse_id;
    param.wait_for_completion = ex_data->offload.wait_for_completion;

    if((ctx = BN_CTX_new()) == NULL) {
        WARN("Failed to allocate BN ctx");
        return ret;
    }

    BN_CTX_start(ctx);
    m = BN_CTX_get(ctx);
    order = BN_CTX_get(ctx);
    if(!m || !order) {
        WARN("Failed to allocate m or order parameters");
        goto cleanup_ctx;
    }

    if(!EC_GROUP_get_order(group, order, ctx)) {
        WARN("Failed to get order from the group");
        goto cleanup_ctx;
    }
    bits_len = BN_num_bits(order);
    ceil_bytes_len = (bits_len + 7) / 8;

    /*
     * Need to truncate digest if it is too long
     */
    hash_input = dgst;
    if (mem_method) {
        hash_input = mem_method->read((PSE_OFFLOAD_MEM *)dgst);
    }
    if (pse_digest_truncate(hash_input, dgst_len, bits_len, m, &dsgt_trunc) <= 0) {
        goto cleanup_ctx;
    }

    if (mem_method) {
        param.sig_input_vec = (uint8_t *)sig;
        param.r = (uint8_t *)ex_data->offload.verify.sig_r;
        param.s = (uint8_t *)ex_data->offload.verify.sig_s;
        if (dsgt_trunc) {
            if (BN_bn2binpad(m, mem_method->read((PSE_OFFLOAD_MEM *)dgst),
                             ceil_bytes_len) <= 0) {
                WARN("Failed BN_bn2binpad");
                goto cleanup_ctx;
            }
            mem_method->content_size_set((PSE_OFFLOAD_MEM *)dgst,
                                         ceil_bytes_len);
            mem_method->write_thru((PSE_OFFLOAD_MEM *)dgst);
        }

    } else {
        if((pub_key = EC_KEY_get0_public_key(eckey)) == NULL) {
            WARN("Invalid pub_key");
            goto cleanup_ctx;
        }
        
        if ((ec_point = EC_GROUP_get0_generator(group)) == NULL) {
            WARN("Failed to retrieve ec_point");
            goto cleanup_ctx;
        }

        p = BN_CTX_get(ctx);
        a = BN_CTX_get(ctx);
        b = BN_CTX_get(ctx);
        xg = BN_CTX_get(ctx);
        yg = BN_CTX_get(ctx);
        xp = BN_CTX_get(ctx);
        yp = BN_CTX_get(ctx);

        siglen = BN_num_bytes(order);

        ECDSA_SIG_get0(sig, &sig_r, &sig_s);
        if(BN_is_zero(sig_r) || BN_is_negative(sig_r) || BN_ucmp(sig_r, order) >= 0 ||
           BN_is_zero(sig_s) || BN_is_negative(sig_s) || BN_ucmp(sig_s, order) >= 0) {
            WARN("sig_r and sig_s validation failed");
            ret = 0;
            goto cleanup_ctx;
        }
        
        if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) ==
                NID_X9_62_prime_field) {
            if(!EC_GROUP_get_curve_GFp(group, p, a, b, ctx)) {
                WARN("Failed to get GFp parameters");
                goto cleanup_ctx;
            } 
            if(!EC_POINT_get_affine_coordinates_GFp(group, ec_point,
                                                    xg, yg, ctx)) {
                WARN("Failed to get ec_point GFp parameters");
                goto cleanup_ctx;
            }
            if(!EC_POINT_get_affine_coordinates_GFp(group, pub_key,
                                                    xp, yp, ctx)) {
                WARN("Failed to get pub_key GFp parameters");
                goto cleanup_ctx;
            }
        } else { 
            if(!EC_GROUP_get_curve_GF2m(group, p, a, b, ctx)) {
                WARN("Failed to get GF2m parameters");
                goto cleanup_ctx;
            } 
            if(!EC_POINT_get_affine_coordinates_GF2m(group, ec_point,
                                                     xg, yg, ctx)) {
                WARN("Failed to get ec_point GF2m parameters");
                goto cleanup_ctx;
            }
            if(!EC_POINT_get_affine_coordinates_GF2m(group, pub_key,
                                                     xp, yp, ctx)) {
                WARN("Failed to get pub_key GF2m parameters");
                goto cleanup_ctx;
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

        param.p  = bp.data;
        param.n  = bn.data;
        param.xg = bxg.data;
        param.yg = byg.data;
        param.a  = ba.data;
        param.b  = bb.data;
        param.xq = bxq.data;
        param.yq = byq.data;
        param.r  = br.data;
        param.s  = bs.data;
    }

#ifndef NO_PEN_HW_OFFLOAD
    ret = offload_method->verify(ex_data->caller_ctx, &param);
    
#else
    ret = 1;
#endif
    INFO("Return value: ret %d", ret);

    if (!mem_method) {
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
    }
cleanup_ctx:
    if(ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    return ret;
}

static EC_KEY *
pse_ec_get_ec_key(const PSE_KEY *key)
{
    EC_KEY *eckey = NULL;

    if(!key)
        return NULL;

    if (key->u.ec_key.offload.curve_nid == NID_undef) {
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
    } else {
        eckey = EC_KEY_new_by_curve_name(key->u.ec_key.offload.curve_nid);
        if(!eckey) {
            WARN("Failed to allocate eckey for nid %d",
                 key->u.ec_key.offload.curve_nid);
            return NULL;
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
    pse_ec_set_ex_data(eckey, key, callback_data);

    return pkey;

err:
    if(eckey) {
        EC_KEY_free(eckey);
    }
    return NULL;
}
