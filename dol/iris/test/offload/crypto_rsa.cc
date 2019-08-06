#include "crypto_rsa.hpp"
#include "hal_if.hpp"
#include "utils.hpp"

/*
 * Rivest, Shamir & Adler algorithm
 */
namespace crypto_rsa {

/*
 * Asym DMA descriptors are referenced by key when needed so
 * a range of representative values below
 */
enum {
    RSA_DMA_DESC_IDX_KEY0,
    RSA_DMA_DESC_IDX_KEY1,
    RSA_DMA_DESC_IDX_HASH_INPUT,
    RSA_DMA_DESC_IDX_SIG_OUTPUT,
    RSA_DMA_DESC_IDX_PLAIN_INPUT,
    RSA_DMA_DESC_IDX_CIPHERED_OUTPUT,
    RSA_DMA_DESC_IDX_CIPHERED_INPUT,
    RSA_DMA_DESC_IDX_PLAIN_OUTPUT,
    RSA_DMA_DESC_IDX_SIG_HASH_INPUT,
    RSA_DMA_DESC_IDX_MAX,
};

/*
 * Constructor
 */
rsa_t::rsa_t(rsa_params_t& params) :
    rsa_params(params),
    evp_md(nullptr),
    key_idx(params.key_idx()),
    hw_started(false),
    test_success(true)
{
    crypto_asym::dma_desc_pool_params_t dma_params;

    assert(params.acc_ring());
    dma_desc_pool = new crypto_asym::dma_desc_pool_t(
                        dma_params.dma_desc_mem_type(params.dma_desc_mem_type()).
                                   num_descs(RSA_DMA_DESC_IDX_MAX).
                                   base_params(params.base_params()));
    status = new crypto_asym::status_t(params.status_mem_type());
    digest = new dp_mem_t(1, EVP_MAX_MD_SIZE,
                          DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                          0, DP_MEM_ALLOC_FILL_ZERO);
    digest_padded = new dp_mem_t(1, CRYPTO_RSA_DIGEST_PADDED_SIZE_MAX,
                                 DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                                 0, DP_MEM_ALLOC_FILL_ZERO);
}


/*
 * Destructor
 */
rsa_t::~rsa_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    rsa_params.base_params().destructor_free_buffers());
    if (rsa_params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (digest_padded) delete digest_padded;
            if (digest) delete digest;
            if (status) delete status;
            if (dma_desc_pool) delete dma_desc_pool;
            key_destroy();
        }
    }
}


/*
 * Generate a private key (if necessary) then invoke engine i/f to hash the
 * argument msg to create a digest before submitting it to engine i/f for
 * signature generation. Note that the latter will cause engine i/f to
 * proceed to invoking rsa_t::push(rsa_push_params_t) below.
 */
bool
rsa_t::pre_push(rsa_pre_push_params_t& pre_params)
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

    evp_md = eng_if::digest_gen(digest_params.hash_algo(pre_params.hash_algo()).
                                              msg(pre_params.msg()).
                                              digest(digest));
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
 * rsa_t::push() with HW params down below.
 */
bool
rsa_t::push(rsa_push_params_t& push_params)
{
    eng_if::rsa_sign_params_t           sign_params;
    eng_if::rsa_verify_params_t         verify_params;
    bool                                success = false;

    this->push_params = push_params;
    assert(evp_md && crypto_asym::key_idx_is_valid(key_idx));

    switch (pre_params.key_create_type()) {

    case RSA_KEY_CREATE_SIGN:
        success = eng_if::rsa_sign(
                          sign_params.md(evp_md).
                                      pad_mode(pre_params.pad_mode()).
                                      key_idx(key_idx).
                                      n(pre_params.n()).
                                      d_e(pre_params.e()).
                                      digest(digest).
                                      digest_padded(digest_padded).
                                      sig_actual(push_params.sig_actual()).
                                      rsa(this).
                                      failure_expected(push_params.failure_expected()).
                                      wait_for_completion(false));
        break;

    case RSA_KEY_CREATE_VERIFY:
    case RSA_KEY_CREATE_ENCRYPT:

        /*
         * Note: here we use sig_actual to hold the output digest_padded
         * to ensure it is contiguous with sig_expected (as both sig_expected
         * and sig_actual come from the same sig_vec)
         */
        success = eng_if::rsa_verify(
                          verify_params.md(evp_md).
                                        pad_mode(pre_params.pad_mode()).
                                        key_idx(key_idx).
                                        n(pre_params.n()).
                                        d_e(pre_params.e()).
                                        sig_expected(push_params.sig_expected()).
                                        digest(digest).
                                        digest_padded(push_params.sig_actual()).
                                        rsa(this).
                                        failure_expected(push_params.failure_expected()).
                                        wait_for_completion(true));
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
rsa_t::push(rsa_hw_sign_params_t& hw_sign_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_sign_params = hw_sign_params;

    /*
     * Initialize status for later polling
     */
    status->init();

    dma_params.desc_idx(RSA_DMA_DESC_IDX_HASH_INPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_sign_params.hash_input());
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params));

    dma_params.desc_idx(RSA_DMA_DESC_IDX_SIG_OUTPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_sign_params.sig_output());
    req_params.output_list_addr(dma_desc_pool->pre_push(dma_params)).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);
                          
    dma_desc_pool->push();
    rsa_params.acc_ring()->push(req_desc.push(), rsa_params.push_type(),
                                rsa_params.seq_qid());
    return true;
}


/*
 * Invoked from engine i/f to send a request to HW to do public key encryption
 * (for possible signature verification).
 */
bool
rsa_t::push(rsa_hw_enc_params_t& hw_enc_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_enc_params = hw_enc_params;

    /*
     * Initialize status for later polling
     */
    status->init();

    dma_params.desc_idx(RSA_DMA_DESC_IDX_PLAIN_INPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_enc_params.plain_input());
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params));

    dma_params.desc_idx(RSA_DMA_DESC_IDX_CIPHERED_OUTPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_enc_params.ciphered_output());
    req_params.output_list_addr(dma_desc_pool->pre_push(dma_params)).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);
                          
    dma_desc_pool->push();
    rsa_params.acc_ring()->push(req_desc.push(), rsa_params.push_type(),
                                rsa_params.seq_qid());
    return true;
}


/*
 * Invoked from engine i/f to send a request to HW to do private key decryption
 */
bool
rsa_t::push(rsa_hw_dec_params_t& hw_dec_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_dec_params = hw_dec_params;

    /*
     * Initialize status for later polling
     */
    status->init();

    dma_params.desc_idx(RSA_DMA_DESC_IDX_CIPHERED_INPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_dec_params.ciphered_input());
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params));

    dma_params.desc_idx(RSA_DMA_DESC_IDX_PLAIN_OUTPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_dec_params.plain_output());
    req_params.output_list_addr(dma_desc_pool->pre_push(dma_params)).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);
                          
    dma_desc_pool->push();
    rsa_params.acc_ring()->push(req_desc.push(), rsa_params.push_type(),
                                rsa_params.seq_qid());
    return true;
}

/*
 * Invoked from engine i/f to send a request to HW to verify a signature.
 * Only works with signatures for digest messages that were not 
 * Openssl encoded and padded. Otherwise, caller should use the 
 * push() method with rsa_hw_enc_params_t above.
 */
bool
rsa_t::push(rsa_hw_verify_params_t& hw_verify_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;

    this->hw_verify_params = hw_verify_params;

    /*
     * Initialize status for later polling
     */
    status->init();

    /*
     * hw_verify_params.sig_input and hw_verify_params.hash_input
     * are contiguous fragments of the same sig_vec so we ask dma_params
     * to double the input size.
     */
    dma_params.desc_idx(RSA_DMA_DESC_IDX_SIG_HASH_INPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(hw_verify_params.sig_input()).
               double_size(true);
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params)).
               status_addr(status->pa()).
               key_idx(key_idx);
    req_desc.pre_push(req_params);
                          
    dma_desc_pool->push();
    rsa_params.acc_ring()->push(req_desc.push(), rsa_params.push_type(),
                                rsa_params.seq_qid());
    return true;
}


/*
 * HW trigger
 */
bool
rsa_t::post_push(void)
{
    rsa_params.acc_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool
rsa_t::completion_check(void)
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
                               "rsa_t status is busy");
        return false;
    }
    test_success = status->success_check(failure_expected);
    return test_success;
}


/*
 * Test result full verification
 */
bool
rsa_t::full_verify(void)
{
    if (!completion_check()) {
        return false;
    }

    if (rsa_key_create_type_is_sign(pre_params.key_create_type())) {
        test_success = expected_actual_verify("signature",
                                              push_params.sig_expected(),
                                              push_params.sig_actual());
    }
    return test_success;
}


/*
 * Create an asymmetric private or public key for:
 * modulus n and exponent d (private) or e (public).
 */
bool
rsa_t::key_create(rsa_pre_push_params_t& pre_params)
{
    crypto_asym::key_desc_t                 key_desc;
    crypto_asym::key_desc_pre_push_params_t key_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;
    dp_mem_t                                *d_e;
    uint32_t                                asym_key_idx;


    if (!eng_if::dp_mem_pad_in_place(pre_params.n(),
                                     pre_params.n()->line_size_get())) {
        OFFL_FUNC_ERR("failed to pad modulus n");
        return false;
    }
    switch (pre_params.key_create_type()) {

    case RSA_KEY_CREATE_SIGN:
        d_e = pre_params.d();
        key_params.cmd_rsa_sign(true);
        break;

    case RSA_KEY_CREATE_VERIFY:
        d_e = pre_params.e();
        key_params.cmd_rsa_verify(true);
        break;

    case RSA_KEY_CREATE_ENCRYPT:
        d_e = pre_params.e();
        key_params.cmd_rsa_encrypt(true);
        break;

    case RSA_KEY_CREATE_DECRYPT:
        d_e = pre_params.d();
        key_params.cmd_rsa_decrypt(true);
        break;

    default:
        OFFL_FUNC_ERR("invalid key_create_type {}",
                      pre_params.key_create_type());
        return false;
        break;
    }

    if (!eng_if::dp_mem_pad_in_place(d_e, d_e->line_size_get())) {
        OFFL_FUNC_ERR("failed to pad d_e");
        return false;
    }

    /*
     * n and d_e each takes a DMA descriptor;
     * key is described by DMA descriptor list;
     */
    dma_params.desc_idx(RSA_DMA_DESC_IDX_KEY1).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(d_e);
    dma_desc_pool->pre_push(dma_params);

    dma_params.desc_idx(RSA_DMA_DESC_IDX_KEY0).
               next_idx(RSA_DMA_DESC_IDX_KEY1).
               data(pre_params.n());

    key_params.key_param_list(dma_desc_pool->pre_push(dma_params)).
               swap_bytes(true).
               cmd_size(d_e->content_size_get());
    key_desc.pre_push(key_params);
    if (hal_if::get_asym_key_index((const uint8_t *)key_desc.push(),
                                   key_desc.size_get(), &asym_key_idx)) {
        OFFL_FUNC_ERR("failed to create asym key");
        return false;
    }

    key_idx = asym_key_idx;
    OFFL_FUNC_DEBUG("created asym key_idx {} for modulus size {}",
                    key_idx, d_e->content_size_get());
    return true;
}

/*
 * Destroy the current asymmetric key.
 */
void
rsa_t::key_destroy(void)
{
    if (!rsa_params.key_idx_shared() &&
        crypto_asym::key_idx_is_valid(key_idx)) {

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
rsa_t::expected_actual_verify(const char *entity_name,
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
 * Report test paramaters for debugging purposes.
 */
void
rsa_t::test_params_report(void)
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
    dp_mem_dump("modulus n", pre_params.n());
    dp_mem_dump("        d", pre_params.d());
    dp_mem_dump("        e", pre_params.e());
    OFFL_LOG_DEBUG("  SHAalgo {}", pre_params.hash_algo());
    dp_mem_dump("      Msg", pre_params.msg());
    dp_mem_dump("      Sig", push_params.sig_expected());
    OFFL_LOG_DEBUG("   Result {}", push_params.failure_expected() ? "F" : "P");
}


/*
 * Access methods for PSE Openssl engine
 */
extern "C" {

static int
sign(void *ctx,
     const PSE_RSA_SIGN_PARAM *param)
{
    rsa_t                   *crypto_rsa = static_cast<rsa_t *>(ctx);
    dp_mem_t                *hash_input;
    dp_mem_t                *sig_output;
    rsa_hw_sign_params_t    sign_params;
    bool                    success;

    hash_input = static_cast<dp_mem_t *>((void *)param->hash_input);
    sig_output = static_cast<dp_mem_t *>((void *)param->sig_output);

    success = crypto_rsa->push(sign_params.hash_input(hash_input).
                                           sig_output(sig_output));
    if (success) {
        if (param->wait_for_completion) {
            crypto_rsa->post_push();
            success = crypto_rsa->completion_check();
        }
    }

    return success ? sig_output->content_size_get() : -1;
}

static int
encrypt(void *ctx,
        const PSE_RSA_ENCRYPT_PARAM *param)
{
    rsa_t                   *crypto_rsa = static_cast<rsa_t *>(ctx);
    dp_mem_t                *plain_input;
    dp_mem_t                *ciphered_output;
    rsa_hw_enc_params_t     enc_params;
    bool                    success;

    plain_input = static_cast<dp_mem_t *>((void *)param->plain_input);
    ciphered_output = static_cast<dp_mem_t *>((void *)param->ciphered_output);

    success = crypto_rsa->push(enc_params.plain_input(plain_input).
                                          ciphered_output(ciphered_output));
    if (success) {
        if (param->wait_for_completion) {
            crypto_rsa->post_push();
            success = crypto_rsa->completion_check();
            if (success) {
                ciphered_output->read_thru();
            }
        }
    }

    return success ? 1 : -1;
}

static int
decrypt(void *ctx,
        const PSE_RSA_DECRYPT_PARAM *param)
{
    rsa_t                   *crypto_rsa = static_cast<rsa_t *>(ctx);
    dp_mem_t                *ciphered_input;
    dp_mem_t                *plain_output;
    rsa_hw_dec_params_t     dec_params;
    bool                    success;

    ciphered_input = static_cast<dp_mem_t *>((void *)param->ciphered_input);
    plain_output = static_cast<dp_mem_t *>((void *)param->plain_output);

    success = crypto_rsa->push(dec_params.ciphered_input(ciphered_input).
                                          plain_output(plain_output));
    if (success) {
        if (param->wait_for_completion) {
            crypto_rsa->post_push();
            success = crypto_rsa->completion_check();
            if (success) {
                plain_output->read_thru();
            }
        }
    }

    return success ? 1 : -1;
}

const PSE_RSA_OFFLOAD_METHOD pse_rsa_offload_method =
{
    .sign       = sign,
    .encrypt    = encrypt,
    .decrypt    = decrypt,
    .mem_method = &pse_mem_method,
};

} // extern "C"


} // namespace crypto_rsa


