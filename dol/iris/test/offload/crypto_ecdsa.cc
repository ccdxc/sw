#include "crypto_ecdsa.hpp"
#include "hal_if.hpp"
#include "utils.hpp"

/*
 * Elliptic Curve Digital Signature Algorithm
 */
namespace crypto_ecdsa {

/*
 * Asym DMA descriptors are referenced by key when needed so
 * a range of representative values below
 */
enum {
    ECDSA_DMA_DESC_IDX_KEY0,
    ECDSA_DMA_DESC_IDX_KEY1,
    ECDSA_DMA_DESC_IDX_SIGN_K_RANDOM,
    ECDSA_DMA_DESC_IDX_SIGN_DIGEST,
    ECDSA_DMA_DESC_IDX_SIGN_SIG_OUTPUT,
    ECDSA_DMA_DESC_IDX_VERIFY_SIG_INPUT,
    ECDSA_DMA_DESC_IDX_VERIFY_DIGEST,
    ECDSA_DMA_DESC_IDX_MAX,
};

/*
 * Constructor
 */
ecdsa_t::ecdsa_t(ecdsa_params_t& params) :
    ecdsa_params(params),
    evp_md(nullptr),
    crypto_sha(nullptr),
    key_idx(CRYPTO_ASYM_KEY_IDX_INVALID),
    hw_started(false),
    test_success(true)
{
    u_long P_expanded_len = params.P_expanded_len();
    u_long digest_size = max(P_expanded_len, (u_long)EVP_MAX_MD_SIZE);
    crypto_asym::dma_desc_pool_params_t dma_params;
    crypto_sha::sha_params_t            sha_params;

    assert(params.acc_ring());
    dma_desc_pool = new crypto_asym::dma_desc_pool_t(
                        dma_params.dma_desc_mem_type(params.dma_desc_mem_type()).
                                   num_descs(ECDSA_DMA_DESC_IDX_MAX).
                                   base_params(params.base_params()));
    status = new crypto_asym::status_t(params.status_mem_type());
    digest = new dp_mem_t(1, digest_size,
                          DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                          0, DP_MEM_ALLOC_FILL_ZERO);
    sha_params.base_params(params.base_params()).
               msg_desc_mem_type(params.dma_desc_mem_type()).
               msg_digest_mem_type(params.msg_digest_mem_type()).
               status_mem_type(params.status_mem_type()).
               doorbell_mem_type(params.status_mem_type()).
               acc_ring(crypto_symm::mpp0_ring).
               push_type(params.push_type()).
               seq_qid(params.seq_qid());
    /*
     * ECDSA will use HW engine to calculate SHA digests
     */
    crypto_sha = new crypto_sha::sha_t(sha_params);
}


/*
 * Destructor
 */
ecdsa_t::~ecdsa_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    ecdsa_params.base_params().destructor_free_buffers());
    if (ecdsa_params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (digest) delete digest;
            if (status) delete status;
            if (dma_desc_pool) delete dma_desc_pool;
            if (crypto_sha) delete crypto_sha;
            key_destroy();
        }
    }
}


/*
 * Generate a private key (if necessary) then invoke engine i/f to hash the
 * argument msg to create a digest before submitting it to engine i/f for
 * signature generation. Note that the latter will cause engine i/f to
 * proceed to invoking ecdsa_t::push(ecdsa_push_params_t) below.
 */
bool
ecdsa_t::pre_push(ecdsa_pre_push_params_t& pre_params)
{
    eng_if::digest_params_t     digest_params;

    this->pre_params = pre_params;
    hw_started = false;
    test_success = false;

    if (!crypto_asym::key_idx_is_valid(key_idx)) {
        if (!key_create(pre_params)) {
            return false;
        }
    }

    digest_params.msg_is_component(
                  ecdsa_key_create_type_is_sign_component(
                                           pre_params.key_create_type()));
    /*
     * Use HW SHA engine if available, except when msg_is_component
     * which means it was already hashed.
     */
    if (crypto_sha && !digest_params.msg_is_component()) {
        crypto_sha::sha_pre_push_params_t    sha_pre_params;
        crypto_sha::sha_push_params_t        sha_push_params;

        sha_pre_params.crypto_symm_type(crypto_symm::CRYPTO_SYMM_TYPE_SHA).
                       sha_nbytes(crypto_sha::sha_nbytes_find(pre_params.hash_algo()));
        if (!crypto_sha->pre_push(sha_pre_params)) {
            OFFL_FUNC_ERR("failed crypto_sha pre_push");
            return false;
        }
        sha_push_params.msg(pre_params.msg()).
                        md_actual(digest).
                        wait_for_completion(true);
        if (!crypto_sha->push(sha_push_params)) {
            OFFL_FUNC_ERR("failed crypto_sha push");
            return false;
        }
        crypto_sha->post_push();
        evp_md = sha_push_params.ret_evp_md();

    } else {
        evp_md = eng_if::digest_gen(digest_params.hash_algo(pre_params.hash_algo()).
                                                  msg(pre_params.msg()).
                                                  digest(digest));
    }
    if (!evp_md) {
        OFFL_FUNC_ERR("failed msg_digest push");
        return false;
    }

    hw_started = true;
    return true;
}


/*
 * Invoke engine i/f to generate a signature. 
 * Note engine i/f will then proceed to invoking 
 * ecdsa_t::push() with HW params down below.
 */
bool
ecdsa_t::push(ecdsa_push_params_t& push_params)
{
    eng_if::ec_sign_params_t            sign_params;
    eng_if::ec_verify_params_t          verify_params;
    bool                                success = false;

    this->push_params = push_params;
    assert(evp_md && crypto_asym::key_idx_is_valid(key_idx));

    switch (pre_params.key_create_type()) {

    case ECDSA_KEY_CREATE_SIGN:
    case ECDSA_KEY_CREATE_SIGN_COMPONENT:
        success = eng_if::ec_sign(
                          sign_params.curve_nid(pre_params.curve_nid()).
                                      md(evp_md).
                                      key_idx(key_idx).
                                      k_random(push_params.k_random()).
                                      digest(digest).
                                      sig_output_vec(push_params.sig_actual_vec()).
                                      ec(this).
                                      skip_DER_encode(true).
                                      failure_expected(push_params.failure_expected()).
                                      wait_for_completion(false));
        break;

    case ECDSA_KEY_CREATE_VERIFY:
        success = eng_if::ec_verify(
                          verify_params.curve_nid(pre_params.curve_nid()).
                                        md(evp_md).
                                        key_idx(key_idx).
                                        digest(digest).
                                        sig_input_vec(push_params.sig_expected_vec()).
                                        sig_r(push_params.r_expected()).
                                        sig_s(push_params.s_expected()).
                                        ec(this).
                                        skip_DER_decode(true).
                                        failure_expected(push_params.failure_expected()).
                                        wait_for_completion(false));
        break;

    default:
        OFFL_FUNC_ERR("unsupported key_create_type {}",
                      pre_params.key_create_type());
        break;
    }

    if (!success && !push_params.failure_expected()) {
        OFFL_FUNC_DEBUG("failed result but should have been successful instead");
        test_params_report();
    }
    return success;
}


/*
 * Invoked from engine i/f to send a request to HW to generate a signature
 * given a (padded) message digest.
 */
bool
ecdsa_t::push(ecdsa_hw_sign_params_t& hw_sign_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_sign_params = hw_sign_params;

    /*
     * Pad out k and digest to the required HW expanded size,
     * which also equals k's line size.
     */
    eng_if::dp_mem_pad_in_place(hw_sign_params.k_random(),
                                hw_sign_params.k_random()->line_size_get());
    eng_if::dp_mem_pad_in_place(hw_sign_params.hash_input(),
                                hw_sign_params.k_random()->line_size_get());

    /*
     * Initialize status for later polling
     */
    status->init();

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_SIGN_DIGEST).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_sign_params.hash_input());
    dma_desc_pool->pre_push(dma_params);

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_SIGN_K_RANDOM).
               next_idx(ECDSA_DMA_DESC_IDX_SIGN_DIGEST).
               data(hw_sign_params.k_random());
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params));

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_SIGN_SIG_OUTPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_sign_params.sig_output_vec());
    req_params.output_list_addr(dma_desc_pool->pre_push(dma_params)).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);

    dma_desc_pool->push();
    ecdsa_params.acc_ring()->push(req_desc.push(), ecdsa_params.push_type(),
                                  ecdsa_params.seq_qid());
    return true;
}


/*
 * Invoked from engine i/f to send a request to HW to verify a signature.
 */
bool
ecdsa_t::push(ecdsa_hw_verify_params_t& hw_verify_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_verify_params = hw_verify_params;

    /*
     * Pad out r_expected and s_expected to the required HW expanded size,
     */
    eng_if::dp_mem_pad_in_place(hw_verify_params.sig_r(),
                                hw_verify_params.sig_r()->line_size_get());
    eng_if::dp_mem_pad_in_place(hw_verify_params.sig_s(),
                                hw_verify_params.sig_s()->line_size_get());
    hw_verify_params.sig_input_vec()->content_size_set(
                         hw_verify_params.sig_r()->content_size_get() * 2);
    /*
     * Pad out digest to the same size
     */
    eng_if::dp_mem_pad_in_place(hw_verify_params.hash_input(),
                                hw_verify_params.sig_r()->line_size_get());
    /*
     * Initialize status for later polling
     */
    status->init();

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_VERIFY_DIGEST).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_verify_params.hash_input());
    dma_desc_pool->pre_push(dma_params);

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_VERIFY_SIG_INPUT).
               next_idx(ECDSA_DMA_DESC_IDX_VERIFY_DIGEST).
               data(hw_verify_params.sig_input_vec());

    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params)).
               output_list_addr(0).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);

    dma_desc_pool->push();
    ecdsa_params.acc_ring()->push(req_desc.push(), ecdsa_params.push_type(),
                                  ecdsa_params.seq_qid());
    return true;
}


/*
 * HW trigger
 */
bool
ecdsa_t::post_push(void)
{
    ecdsa_params.acc_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool
ecdsa_t::completion_check(void)
{
    bool    failure_expected = push_params.failure_expected();

    auto status_busy_check = [this] () -> int
    {
        if (status->busy_check()) {
            return 1;
        }
        return 0;
    };

#ifdef __x86_64__
    utils::Poller poll(failure_expected ? poll_interval() / 10 :
                                          long_poll_interval());
#else
    utils::Poller poll;
#endif
    test_success = false;
    if (poll(status_busy_check, failure_expected)) {
        OFFL_FUNC_ERR_OR_DEBUG(failure_expected,
                               "ecdsa_t status is busy");
        return false;
    }
    test_success = status->success_check(failure_expected);
    return test_success;
}


/*
 * Test result full verification
 */
bool
ecdsa_t::full_verify(void)
{
    if (!completion_check()) {
        return false;
    }

    if (ecdsa_key_create_type_is_sign(pre_params.key_create_type())) {

        /*
         * Unpad the actual signatures before verification. HW has a known
         * yet-to-be-explained behavior where it may pad the resulting
         * signatures with non-zero bytes!
         */
        eng_if::dp_mem_unpad_in_place(push_params.r_actual(),
                                      ecdsa_params.P_bytes_len());
        eng_if::dp_mem_unpad_in_place(push_params.s_actual(),
                                      ecdsa_params.P_bytes_len());
        test_success = expected_actual_verify("signature (r)",
                                              push_params.r_expected(),
                                              push_params.r_actual()) &&
                       expected_actual_verify("signature (s)",
                                              push_params.s_expected(),
                                              push_params.s_actual());
    }
    return test_success;
}


/*
 * Create an asymmetric private or public key for:
 * modulus n and exponent d (private) or e (public).
 */
bool
ecdsa_t::key_create(ecdsa_pre_push_params_t& pre_params)
{
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;
    crypto_asym::key_desc_pre_push_params_t      key_params;
    crypto_asym::key_desc_t                      key_desc;
    uint32_t                                     asym_key_idx;

    switch (pre_params.key_create_type()) {

    case ECDSA_KEY_CREATE_SIGN:
    case ECDSA_KEY_CREATE_SIGN_COMPONENT:
        key_params.cmd_ecdsa_sign(true);

        /*
         * Pad out d to the required HW expanded size,
         */
        eng_if::dp_mem_pad_in_place(pre_params.d(),
                                    pre_params.d()->line_size_get());
        /*
         * curve domain params and d each takes a DMA descriptor;
         * key is described by DMA descriptor list;
         */
        dma_params.desc_idx(ECDSA_DMA_DESC_IDX_KEY1).
                   next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
                   data(pre_params.d());
        dma_desc_pool->pre_push(dma_params);
        break;

    case ECDSA_KEY_CREATE_VERIFY:
        key_params.cmd_ecdsa_verify(true);

        /*
         * Pad out (qx, qy) to the required HW expanded size;
         * Both of these are fragments of pre_params.pub_key_vec()
         */
        eng_if::dp_mem_pad_in_place(pre_params.qx(),
                                    pre_params.qx()->line_size_get());
        eng_if::dp_mem_pad_in_place(pre_params.qy(),
                                    pre_params.qy()->line_size_get());
        pre_params.pub_key_vec()->content_size_set(
                                  pre_params.qx()->content_size_get() * 2);
        /*
         * curve domain params and d each takes a DMA descriptor;
         * key is described by DMA descriptor list;
         */
        dma_params.desc_idx(ECDSA_DMA_DESC_IDX_KEY1).
                   next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
                   data(pre_params.pub_key_vec());
        dma_desc_pool->pre_push(dma_params);
        break;

    default:
        OFFL_FUNC_ERR("invalid key_create_type {}",
                      pre_params.key_create_type());
        return false;
        break;
    }

    dma_params.desc_idx(ECDSA_DMA_DESC_IDX_KEY0).
               next_idx(ECDSA_DMA_DESC_IDX_KEY1).
               data(pre_params.domain_vec());
    /*
     * Note that HW expects cmd_size to reflect the real P_bytes_len
     * and not the P_expanded_len.
     */
    key_params.key_param_list(dma_desc_pool->pre_push(dma_params)).
               swap_bytes(true).
               cmd_size(ecdsa_params.P_bytes_len());
    key_desc.pre_push(key_params);
    if (hal_if::get_asym_key_index((const uint8_t *)key_desc.push(),
                                   key_desc.size_get(), &asym_key_idx)) {
        OFFL_FUNC_ERR("failed asym key for domain_vec size {} key size {}",
                      pre_params.domain_vec()->content_size_get(),
                      ecdsa_params.P_bytes_len());
        return false;
    }

    key_idx = asym_key_idx;
    OFFL_FUNC_DEBUG("created asym key_idx {} for domain_vec size {} key size {}",
                    key_idx, pre_params.domain_vec()->content_size_get(),
                    ecdsa_params.P_bytes_len());
    return true;
}

/*
 * Destroy the current asymmetric key.
 */
void
ecdsa_t::key_destroy(void)
{
    if (crypto_asym::key_idx_is_valid(key_idx)) {

        OFFL_FUNC_DEBUG("destroying asym key_idx {}", key_idx);
        if (hal_if::delete_asym_key(key_idx)) {
            OFFL_FUNC_ERR("failed to destroy asym key_idx {}", key_idx);
        }
        key_idx = CRYPTO_ASYM_KEY_IDX_INVALID;
    }
}


/*
 * Verify expected vs. actual
 */
bool
ecdsa_t::expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual)
{
    if (expected->content_size_get() != actual->content_size_get()) {
        OFFL_FUNC_ERR("expected {} size {} actual size {}",
                      entity_name, expected->content_size_get(),
                      actual->content_size_get());
        return false;
    }

    if (memcmp(expected->read_thru(), actual->read_thru(),
               expected->content_size_get())) {
        if (OFFL_IS_LOG_LEVEL_DEBUG()) {
            OFFL_FUNC_DEBUG("expected {}", entity_name);
            utils::dump(expected->read(), expected->content_size_get());
            OFFL_FUNC_DEBUG("actual {}", entity_name);
            utils::dump(actual->read(), actual->content_size_get());
        }
        return false;
    }

    OFFL_FUNC_DEBUG("successful match for expected and actual {}s (size was {})",
                    entity_name, actual->content_size_get());
    return true;
}

/*
 * Report test parameters for debugging purposes.
 */
void
ecdsa_t::test_params_report(void)
{
    auto dp_mem_dump = [] (const char *name,
                           dp_mem_t *mem) -> void
    {
        if (mem) {
            OFFL_LOG_DEBUG("{} size {}", name, mem->content_size_get());
            utils::dump(mem->read(), mem->content_size_get());
        }
    };

    if (!OFFL_IS_LOG_LEVEL_DEBUG()) {
        return;
    }
    OFFL_LOG_DEBUG("Test params");
    dp_mem_dump("        d", pre_params.d());
    dp_mem_dump("      Msg", pre_params.msg());
    dp_mem_dump("        r", push_params.r_expected());
    dp_mem_dump("        s", push_params.s_expected());
    OFFL_LOG_DEBUG("   Result {}", push_params.failure_expected() ? "F" : "P");
}

/*
 * Access methods for PSE Openssl engine
 */
extern "C" {

static int
sign(void *ctx,
     const PSE_EC_SIGN_PARAM *param)
{
    ecdsa_t                 *crypto_ecdsa = static_cast<ecdsa_t *>(ctx);
    dp_mem_t                *k_random;
    dp_mem_t                *hash_input;
    dp_mem_t                *sig_output_vec;
    ecdsa_hw_sign_params_t  sign_params;
    bool                    success;

    k_random = static_cast<dp_mem_t *>((void *)param->k_random);
    hash_input = static_cast<dp_mem_t *>((void *)param->hash_input);
    sig_output_vec = static_cast<dp_mem_t *>((void *)param->sig_output_vec);

    success = crypto_ecdsa->push(sign_params.k_random(k_random).
                                             hash_input(hash_input).
                                             sig_output_vec(sig_output_vec));
    if (success) {
        if (param->wait_for_completion) {
            crypto_ecdsa->post_push();
            success = crypto_ecdsa->completion_check();
        }
    }

    return success ? sig_output_vec->line_size_get() : -1;
}

static int
verify(void *ctx,
       const PSE_EC_VERIFY_PARAM *param)
{
    ecdsa_t                 *crypto_ecdsa = static_cast<ecdsa_t *>(ctx);
    dp_mem_t                *hash_input;
    dp_mem_t                *sig_intput_vec;
    dp_mem_t                *sig_r;
    dp_mem_t                *sig_s;
    ecdsa_hw_verify_params_t verify_params;
    bool                    success;

    hash_input = static_cast<dp_mem_t *>((void *)param->hash_input);
    sig_intput_vec = static_cast<dp_mem_t *>((void *)param->sig_input_vec);
    sig_r = static_cast<dp_mem_t *>((void *)param->r);
    sig_s = static_cast<dp_mem_t *>((void *)param->s);

    success = crypto_ecdsa->push(verify_params.hash_input(hash_input).
                                               sig_input_vec(sig_intput_vec).
                                               sig_r(sig_r).
                                               sig_s(sig_s));
    if (success) {
        if (param->wait_for_completion) {
            crypto_ecdsa->post_push();
            success = crypto_ecdsa->completion_check();
        }
    }

    return success ? 1 : -1;
}

const PSE_EC_OFFLOAD_METHOD pse_ec_offload_method =
{
    .sign       = sign,
    .verify     = verify,
    .mem_method = &pse_mem_method,
};

} // extern "C"


} // namespace crypto_ecdsa
