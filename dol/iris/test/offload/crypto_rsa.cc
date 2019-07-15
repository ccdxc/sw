#include "crypto_rsa.hpp"
#include "hal_if.hpp"
#include "utils.hpp"

namespace crypto_rsa {

/*
 * Asym DMA descriptors are referenced by key when needed so
 * a range of representative values below
 */
enum {
    RSA_DMA_DESC_IDX_KEY0,
    RSA_DMA_DESC_IDX_KEY1,
    RSA_DMA_DESC_IDX_DIGEST,
    RSA_DMA_DESC_IDX_SIG_OUTPUT,
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
    asym_status = new dp_mem_t(1, sizeof(crypto_asym::status_t),
                               DP_MEM_ALIGN_NONE, params.status_mem_type(),
                               0, DP_MEM_ALLOC_NO_FILL);
    msg_digest = new dp_mem_t(1, EVP_MAX_MD_SIZE,
                              DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                              0, DP_MEM_ALLOC_NO_FILL);
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
            if (msg_digest) delete msg_digest;
            if (asym_status) delete asym_status;
            if (dma_desc_pool) delete dma_desc_pool;
            key_destroy();
        }
    }
}


/*
 * Generate a private key (if necessary) then invoke DOLE to hash the
 * argument msg to create a digest before submitting it to DOLE for
 * signature generation. Note that the latter will cause DOLE to
 * proceed to invoking rsa_t::push(rsa_sig_push_params_t) below.
 */
bool
rsa_t::pre_push(rsa_sig_pre_push_params_t& sig_pre_params)
{
    this->sig_pre_params = sig_pre_params;
    hw_started = false;
    test_success = false;

    if (!crypto_asym::key_idx_is_valid(key_idx)) {
        if (!key_create(sig_pre_params)) {
            return false;
        }
    }

    evp_md = dole_if::rsa_msg_digest_push(sig_pre_params.hash_algo(),
                                          sig_pre_params.msg(),
                                          msg_digest);
    if (!evp_md) {
        OFFL_FUNC_ERR("failed msg_digest push");
        return false;
    }

    hw_started = true;
    return true;
}


/*
 * Invoke DOLE to generate a signature. 
 * Note DOLE will then proceed to invoking 
 * rsa_t::push(rsa_sig_padded_push_params_t) below.
 */
bool
rsa_t::push(rsa_sig_push_params_t& sig_push_params)
{
    this->sig_push_params = sig_push_params;

    assert(evp_md && crypto_asym::key_idx_is_valid(key_idx));
    return dole_if::rsa_sig_push(this, evp_md, key_idx,
                                 sig_pre_params.n(),
                                 sig_pre_params.e(),
                                 msg_digest,
                                 sig_push_params.sig_actual());
}


/*
 * Invoked from DOLE to send a request to HW to generate a signature
 * given a (padded) message digest.
 */
bool
rsa_t::push(rsa_sig_padded_push_params_t& sig_padded_params)
{
    crypto_asym::req_desc_t                 req_desc;
    crypto_asym::req_desc_pre_push_params_t req_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;
    crypto_asym::status_t                   *status;

    this->sig_padded_params = sig_padded_params;

    dma_params.desc_idx(RSA_DMA_DESC_IDX_DIGEST).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(sig_padded_params.digest_padded());
    req_params.input_list_addr(dma_desc_pool->pre_push(dma_params));

    dma_params.desc_idx(RSA_DMA_DESC_IDX_SIG_OUTPUT).
               next_idx(CRYPTO_ASYM_DMA_DESC_IDX_INVALID).
               data(sig_padded_params.sig_actual());
    req_params.output_list_addr(dma_desc_pool->pre_push(dma_params));

    /*
     * Initialize status for later polling
     */
    asym_status->clear();
    status = (crypto_asym::status_t *)asym_status->read();
    status->pk_busy = true;
    asym_status->write_thru();

    req_params.status_addr(asym_status->pa()).
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
 * Test result verification (fast and non-blocking)
 *
 * Should only be used when caller has another means to ensure that the test
 * has completed. The main purpose of this function to quickly verify operational
 * status, and avoid any lengthy HBM access (such as data comparison) that would
 * slow down test resubmission in the scaled setup.
 */
bool
rsa_t::fast_verify(void)
{
    crypto_asym::status_t *status;

    auto status_busy_check = [this, &status] () -> int {
        status = (crypto_asym::status_t *)asym_status->read_thru();
        if (status->pk_busy) {
            return 1;
        }
        return 0;
    };

    utils::Poller poll;

    test_success = false;
    if (poll(status_busy_check) || status->err_flags) {
        OFFL_FUNC_ERR("rsa_t status err_flags {:#x} fail_addr {:#x}",
                      status->err_flags, status->fail_addr);
        return false;
    }
    test_success = true;
    return true;
}


/*
 * Test result verification (full and possibly blocking)
 *
 * Should only be used in non-scaled setup.
 */
bool
rsa_t::full_verify(void)
{
    dp_mem_t    *sig_expected;
    dp_mem_t    *sig_actual;

    if (!fast_verify()) {
        return false;
    }

    test_success = false;
    sig_expected = sig_push_params.sig_expected();
    sig_actual = sig_push_params.sig_actual();
    if (sig_expected->content_size_get() != sig_actual->content_size_get()) {
        OFFL_FUNC_ERR("expected signature size {} actual size {}",
                      sig_expected->content_size_get(),
                      sig_actual->content_size_get());
        return false;
    }

    if (memcmp(sig_expected->read_thru(), sig_actual->read_thru(),
               sig_expected->content_size_get())) {
        if (OFFL_IS_LOG_LEVEL_DEBUG()) {
            OFFL_FUNC_DEBUG("expected signature");
            utils::dump(sig_expected->read(), sig_expected->content_size_get());
            OFFL_FUNC_DEBUG("actual signature");
            utils::dump(sig_actual->read(), sig_actual->content_size_get());
        }
        return false;
    }

    OFFL_FUNC_DEBUG("expected and actual signatures matched! (size was {})",
                    sig_actual->content_size_get());
    test_success = true;
    return true;
}

/*
 * Create an asymmetric private or public key for:
 * modulus n and exponent d (private) or e (public).
 */
bool
rsa_t::key_create(rsa_sig_pre_push_params_t& sig_pre_params)
{
    crypto_asym::key_desc_t                 key_desc;
    crypto_asym::key_desc_pre_push_params_t key_params;
    crypto_asym::dma_desc_pool_pre_push_params_t dma_params;
    dp_mem_t                                *d_e;
    uint32_t                                modulus_size;
    uint32_t                                asym_key_idx;

    modulus_size = sig_pre_params.n()->content_size_get();
    assert(modulus_size == sig_pre_params.d()->content_size_get());
    assert(modulus_size == sig_pre_params.e()->content_size_get());

    switch (sig_pre_params.key_create_type()) {

    case RSA_KEY_CREATE_PRIV_SIG_GEN:
        d_e = sig_pre_params.d();
        key_params.cmd_rsa_sig_gen(true);
        break;

    case RSA_KEY_CREATE_PRIV_SIG_VERIFY:
        d_e = sig_pre_params.d();
        key_params.cmd_rsa_sig_verify(true);
        break;

    case RSA_KEY_CREATE_PUB_SIG_GEN:
        d_e = sig_pre_params.e();
        key_params.cmd_rsa_sig_gen(true);
        break;

    case RSA_KEY_CREATE_PUB_SIG_VERIFY:
        d_e = sig_pre_params.e();
        key_params.cmd_rsa_sig_verify(true);
        break;

    default:
        OFFL_FUNC_ERR("invalid key_create_type {}",
                      sig_pre_params.key_create_type());
        return false;
        break;
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
               data(sig_pre_params.n());

    key_params.key_param_list(dma_desc_pool->pre_push(dma_params)).
               swap_bytes(true).
               cmd_size(modulus_size);
    key_desc.pre_push(key_params);
    if (hal_if::get_asym_key_index((const uint8_t *)key_desc.push(),
                                   key_desc.size_get(), &asym_key_idx)) {
        OFFL_FUNC_ERR("failed to create asym key");
        return false;
    }

    key_idx = asym_key_idx;
    OFFL_FUNC_DEBUG("created asym key_idx {} for modulus_size {}",
                    key_idx, modulus_size);
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

} // namespace crypto_rsa
