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
    ECDSA_DMA_DESC_IDX_HASH_INPUT,
    ECDSA_DMA_DESC_IDX_SIG_OUTPUT,
    ECDSA_DMA_DESC_IDX_PLAIN_INPUT,
    ECDSA_DMA_DESC_IDX_CIPHERED_OUTPUT,
    ECDSA_DMA_DESC_IDX_CIPHERED_INPUT,
    ECDSA_DMA_DESC_IDX_PLAIN_OUTPUT,
    ECDSA_DMA_DESC_IDX_SIG_HASH_INPUT,
    ECDSA_DMA_DESC_IDX_MAX,
};

/*
 * Constructor
 */
ecdsa_t::ecdsa_t(ecdsa_params_t& params) :
    ecdsa_params(params),
    evp_md(nullptr),
    key_idx(params.key_idx()),
    hw_started(false),
    test_success(true)
{
    crypto_asym::dma_desc_pool_params_t dma_params;

    assert(params.acc_ring());
    dma_desc_pool = new crypto_asym::dma_desc_pool_t(
                        dma_params.dma_desc_mem_type(params.dma_desc_mem_type()).
                                   num_descs(ECDSA_DMA_DESC_IDX_MAX).
                                   base_params(params.base_params()));
    status = new crypto_asym::status_t(params.status_mem_type());
    digest = new dp_mem_t(1, EVP_MAX_MD_SIZE,
                          DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                          0, DP_MEM_ALLOC_FILL_ZERO);
    digest_padded = new dp_mem_t(1, CRYPTO_ECDSA_DIGEST_PADDED_SIZE_MAX,
                                 DP_MEM_ALIGN_NONE, params.msg_digest_mem_type(),
                                 0, DP_MEM_ALLOC_FILL_ZERO);
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
            if (digest_padded) delete digest_padded;
            if (digest) delete digest;
            if (status) delete status;
            if (dma_desc_pool) delete dma_desc_pool;
            key_destroy();
        }
    }
}


/*
 * Generate a private key (if necessary) then invoke DOLE to hash the
 * argument msg to create a digest before submitting it to DOLE for
 * signature generation. Note that the latter will cause DOLE to
 * proceed to invoking ecdsa_t::push(ecdsa_push_params_t) below.
 */
bool
ecdsa_t::pre_push(ecdsa_pre_push_params_t& pre_params)
{
    return true;
}


/*
 * Invoke DOLE to generate a signature. 
 * Note DOLE will then proceed to invoking 
 * ecdsa_t::push() with HW params down below.
 */
bool
ecdsa_t::push(ecdsa_push_params_t& push_params)
{
    return true;
}


/*
 * Invoked from DOLE to send a request to HW to generate a signature
 * given a (padded) message digest.
 */
bool
ecdsa_t::push(ecdsa_hw_sign_params_t& hw_sign_params)
{
    return true;
}


/*
 * Invoked from DOLE to send a request to HW to do public key encryption
 * (for possible signature verification).
 */
bool
ecdsa_t::push(ecdsa_hw_enc_params_t& hw_enc_params)
{
    return true;
}


/*
 * Invoked from DOLE to send a request to HW to do private key decryption
 */
bool
ecdsa_t::push(ecdsa_hw_dec_params_t& hw_dec_params)
{
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

    utils::Poller poll(failure_expected ? poll_interval() / 10 :
                                          long_poll_interval());
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

    return true;
}


/*
 * Create an asymmetric private or public key for:
 * modulus n and exponent d (private) or e (public).
 */
bool
ecdsa_t::key_create(ecdsa_pre_push_params_t& pre_params)
{
    return true;
}

/*
 * Destroy the current asymmetric key.
 */
void
ecdsa_t::key_destroy(void)
{
}


/*
 * Verify expected vs. actual
 */
bool
ecdsa_t::expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual)
{
    return true;
}

/*
 * Report test paramaters for debugging purposes.
 */
void
ecdsa_t::test_params_report(void)
{
}

} // namespace crypto_ecdsa
