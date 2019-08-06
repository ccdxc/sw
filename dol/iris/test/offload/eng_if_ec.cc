#include "eng_if.hpp"
#include "utils.hpp"
#include "crypto_ecdsa.hpp"

namespace eng_if {

/*
 * Establish an EVP public/private key for i/f with openssl.
 * This will invoke the PSE engine function registered with
 * ENGINE_set_load_privkey_function().
 */
static EVP_PKEY *
local_ec_pkey_load(void *caller_ctx,
                   crypto_asym::key_idx_t key_idx,
                   int curve_nid,
                   dp_mem_t *k_random,
                   dp_mem_t *sig_output_vec,
                   bool skip_DER_encode,
                   bool wait_for_completion)
{
    PSE_KEY             key = {0};
    EVP_PKEY            *pkey;

    key.type = EVP_PKEY_EC;
    key.u.ec_key.key_id = key_idx;
    key.u.ec_key.offload.curve_nid = curve_nid;
    key.u.ec_key.offload.offload_method = &crypto_ecdsa::pse_ec_offload_method;
    key.u.ec_key.offload.k_random = (PSE_OFFLOAD_MEM *)k_random;
    key.u.ec_key.offload.sig_output_vec = (PSE_OFFLOAD_MEM *)sig_output_vec;
    key.u.ec_key.offload.skip_DER_encode = skip_DER_encode;
    key.u.ec_key.offload.wait_for_completion = wait_for_completion;

    pkey = ENGINE_load_private_key(eng_if_engine, (const char *)&key,
                                   NULL, caller_ctx);
    if (!pkey) {
        OFFL_FUNC_ERR("Failed to setup EC key");
    }
    return pkey;
}


/*
 * Given a curve type, generate the domain parameters
 * needed for HW key programming.
 */
bool
ec_domain_params_gen(ec_domain_params_t& params)
{
    EC_KEY              *ec_key = NULL;
    BN_CTX              *bn_ctx = NULL;
    const EC_GROUP      *group;
    const EC_POINT      *point;
    BIGNUM              *p, *order, *xg, *yg, *a, *b; // derived from bn_ctx
    int                 nid;
    int                 field_type;
    bool                success = false;

    nid = EC_curve_nist2nid(params.curve_name().c_str());
    if (nid == NID_undef) {
        OFFL_FUNC_ERR("Failed to locate nid for curve_name {}",
                      params.curve_name());
        goto done;
    }
    params.curve_nid(nid);

    ec_key = EC_KEY_new_by_curve_name(nid);
    if (!ec_key) {
        OFFL_FUNC_ERR("Failed to get ec_key for nid {}", nid);
        goto done;
    }

    group = EC_KEY_get0_group(ec_key);
    if (!group) {
        OFFL_FUNC_ERR("Failed to locate group");
        goto done;
    }

    point = EC_GROUP_get0_generator(group);
    if (!point) {
        OFFL_FUNC_ERR("Failed to locate point");
        goto done;
    }

    bn_ctx = BN_CTX_new();
    if (!bn_ctx) {
        OFFL_FUNC_ERR("Failed to allocate BN_ctx");
        goto done;
    }

    BN_CTX_start(bn_ctx);
    p = BN_CTX_get(bn_ctx);
    a = BN_CTX_get(bn_ctx);
    b = BN_CTX_get(bn_ctx);
    xg = BN_CTX_get(bn_ctx);
    yg = BN_CTX_get(bn_ctx);
    order = BN_CTX_get(bn_ctx);

    if (!order || !EC_GROUP_get_order(group, order, bn_ctx)) {
        OFFL_FUNC_ERR("Failed to obtain order");
        goto done;
    }

    field_type = EC_METHOD_get_field_type(EC_GROUP_method_of(group));
    if (field_type == NID_X9_62_prime_field) {
        OFFL_FUNC_DEBUG("key field type prime {}", field_type);
        if ((!EC_GROUP_get_curve_GFp(group, p, a, b, bn_ctx)) ||
            (!EC_POINT_get_affine_coordinates_GFp(group, point,
                                                  xg, yg, bn_ctx))) {
            OFFL_FUNC_ERR("Failed to get curve params for prime field");
            goto done;
        }
    } else {
        OFFL_FUNC_DEBUG("key field type binary {}", field_type);
        if ((!EC_GROUP_get_curve_GF2m(group, p, a, b, bn_ctx)) ||
            (!EC_POINT_get_affine_coordinates_GF2m(group, point,
                                                   xg, yg, bn_ctx))) {
            OFFL_FUNC_ERR("Failed to get curve params for binary");
            goto done;
        }
    }

    bn_to_dp_mem_pad(p, params.p(), 0);
    bn_to_dp_mem_pad(order, params.n(), 0);
    bn_to_dp_mem_pad(xg, params.xg(), 0);
    bn_to_dp_mem_pad(yg, params.yg(), 0);
    bn_to_dp_mem_pad(a, params.a(), 0);
    bn_to_dp_mem_pad(b, params.b(), 0);

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        OFFL_FUNC_DEBUG("p size {}", params.p()->content_size_get());
        utils::dump(params.p()->read(), params.p()->content_size_get());
        OFFL_FUNC_DEBUG("n size {}", params.n()->content_size_get());
        utils::dump(params.n()->read(), params.n()->content_size_get());
        OFFL_FUNC_DEBUG("xg size {}", params.xg()->content_size_get());
        utils::dump(params.xg()->read(), params.xg()->content_size_get());
        OFFL_FUNC_DEBUG("yg size {}", params.yg()->content_size_get());
        utils::dump(params.yg()->read(), params.yg()->content_size_get());
        OFFL_FUNC_DEBUG("a size {}", params.a()->content_size_get());
        utils::dump(params.a()->read(), params.a()->content_size_get());
        OFFL_FUNC_DEBUG("b size {}", params.b()->content_size_get());
        utils::dump(params.b()->read(), params.b()->content_size_get());
        OFFL_FUNC_DEBUG("\n");
    }

    success = true;

done:
    if (!success) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();

    if (bn_ctx) {
        BN_CTX_end(bn_ctx);
        BN_CTX_free(bn_ctx);
    }
    if (ec_key) {
        EC_KEY_free(ec_key);
    }
    return success;
}


/*
 * Generate a signature, i.e., sign an already hashed message
 * given in digest.
 */
bool
ec_sign(ec_sign_params_t& params)
{
    EVP_PKEY_CTX        *pkey_ctx = NULL;
    EVP_PKEY            *evp_pkey = NULL;
    EC_KEY              *ec_key = NULL;
    dp_mem_t            *digest = params.digest();
    dp_mem_t            *sig_output_vec = params.sig_output_vec();
    size_t              siglen;
    int                 ossl_ret;
    bool                success = false;

    /* Setup key */
    evp_pkey = local_ec_pkey_load(params.ec(), params.key_idx(), params.curve_nid(),
                                  params.k_random(), params.sig_output_vec(),
                                  params.skip_DER_encode(),
                                  params.wait_for_completion());
    if (!evp_pkey) {
        OFFL_FUNC_ERR("Failed to setup EC evp_pkey");
        goto done;
    }

    ec_key = EVP_PKEY_get0_EC_KEY(evp_pkey);
    if (!ec_key) {
        OFFL_FUNC_ERR("Failed to locate curve for nid {}", params.curve_nid());
        goto done;
    }

    if (!EVP_PKEY_set1_EC_KEY(evp_pkey, ec_key)) {
        OFFL_FUNC_ERR("Failed EVP_PKEY_set1_EC_KEY");
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

    ossl_ret = EVP_PKEY_sign(pkey_ctx, (unsigned char *)sig_output_vec,
                             &siglen, (const unsigned char *)digest,
                             digest->content_size_get());
    if ((ossl_ret <= 0) && !params.failure_expected()) {
        OFFL_FUNC_ERR("Failed to sign digest message: {}", ossl_ret);
        goto done;
    }

    success = true;

done:
    if (!success && !params.failure_expected()) {
        ERR_print_errors(eng_if_bio);
    }
    ERR_clear_error();
    if (pkey_ctx) {
        EVP_PKEY_CTX_free(pkey_ctx);
    }
    if (evp_pkey) {
        EVP_PKEY_free(evp_pkey);
    }
    if (ec_key) {
        EC_KEY_free(ec_key);
    }
    return success;
}


} // namespace eng_intf
