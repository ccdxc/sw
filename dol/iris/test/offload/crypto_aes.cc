#include "crypto_aes.hpp"
#include "hal_if.hpp"
#include "utils.hpp"

/*
 * Advanced Encyption Standard
 */
namespace crypto_aes {

/*
 * Message descriptor (i.e., AOL) pool, enough to support a dozen
 * or so message input fragments (each AOL can hold 3 fragments).
 */
#define AES_MSG_INPUT_DESC_IDX_MAX      4
#define AES_MSG_OUTPUT_DESC_IDX_MAX     4

/*
 * Constructor
 */
aes_t::aes_t(aes_params_t& params) :
    aes_params(params),
    evp_cipher_ctx(nullptr),
    key_idx(CRYPTO_SYMM_KEY_IDX_INVALID),
    hw_started(false),
    test_success(true)
{
    crypto_symm::msg_desc_pool_params_t msg_params;

    assert(params.acc_ring());
    msg_input_desc_pool = new crypto_symm::msg_desc_pool_t(
                              msg_params.msg_desc_mem_type(params.msg_desc_mem_type()).
                              num_descs(AES_MSG_INPUT_DESC_IDX_MAX).
                              base_params(params.base_params()));
    msg_output_desc_pool = new crypto_symm::msg_desc_pool_t(
                               msg_params.msg_desc_mem_type(params.msg_desc_mem_type()).
                               num_descs(AES_MSG_OUTPUT_DESC_IDX_MAX).
                               base_params(params.base_params()));

    status = new crypto_symm::status_t(params.status_mem_type());
    doorbell = new crypto_symm::doorbell_t(params.doorbell_mem_type(),
                                           0x0ae50ae55ea05ea0);
}


/*
 * Destructor
 */
aes_t::~aes_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    aes_params.base_params().destructor_free_buffers());
    if (aes_params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (doorbell) delete doorbell;
            if (status) delete status;
            if (msg_output_desc_pool) delete msg_output_desc_pool;
            if (msg_input_desc_pool) delete msg_input_desc_pool;
            key_destroy(true);
        }
    }
}


/*
 * Program a key (if necessary).
 */
bool
aes_t::pre_push(aes_pre_push_params_t& pre_params)
{
    eng_if::cipher_init_params_t        init_params;

    this->pre_params = pre_params;
    key_idx = pre_params.key_idx();
    hw_started = false;
    test_success = false;

    init_params.cipher_hw_ctx(this).
                crypto_symm_type(pre_params.crypto_symm_type()).
                key(pre_params.key()).
                iv(pre_params.iv()).
                encrypt(pre_params.op() == crypto_symm::CRYPTO_SYMM_OP_ENCRYPT);
    evp_cipher_ctx = eng_if::cipher_init(init_params);
    if (!evp_cipher_ctx) {
        OFFL_FUNC_ERR("failed cipher_init push");
        return false;
    }
    return true;
}


/*
 * Invoke engine i/f to execute a cipher operation.
 * Note engine will then proceed to invoking 
 * aes_t::push() with HW params down below.
 */
bool
aes_t::push(aes_push_params_t& push_params)
{
    eng_if::cipher_update_params_t  cipher_params;

    assert(evp_cipher_ctx && crypto_symm::key_idx_is_valid(key_idx));
    this->push_params = push_params;
    hw_started = true;

    cipher_params.cipher_hw_ctx(this).
                  cipher_sw_ctx(evp_cipher_ctx).
                  key_idx(key_idx).
                  msg_input(push_params.msg_input()).
                  msg_output(push_params.msg_output_actual()).
                  failure_expected(push_params.failure_expected()).
                  wait_for_completion(push_params.wait_for_completion());

    return eng_if::cipher_update(cipher_params);
}


/*
 * Invoked from PSE engine to initialize a HW cipher operation.
 */
bool
aes_t::push(aes_hw_init_params_t& hw_init_params)
{
    if (!crypto_symm::key_idx_is_valid(key_idx)) {
        if (!key_create(hw_init_params)) {
            return false;
        }
    }
    return true;
}

/*
 * Invoked from PSE engine to incrementally add a msg_input/msg_output.
 */
bool
aes_t::push(aes_hw_cipher_params_t& hw_cipher_params)
{
    crypto_symm::req_desc_t                 req_desc;
    crypto_symm::cmd_pre_push_params_t      cmd_params;
    crypto_symm::req_desc_pre_push_params_t req_params;

    /*
     * We operate as a custom cipher from Openssl's perspective which means
     * all input data are already an integral number of data blocks and
     * no padding would be needed.
     */
    if (hw_cipher_params.msg_input() && hw_cipher_params.msg_output()) {
        msg_input_vec.push_back(hw_cipher_params.msg_input());
        msg_output_vec.push_back(hw_cipher_params.msg_output());

    } else {

        /*
         * Absence of msg_input/msg_output means initiate HW execution.
         */
        crypto_symm::msg_desc_pool_pre_push_params_t 
                                            input_params(msg_input_vec);
        crypto_symm::msg_desc_pool_pre_push_params_t 
                                            output_params(msg_output_vec);
        /*
         * Initialize status for later polling
         */
        status->init();
        doorbell->init();

        cmd_params.type(pre_params.crypto_symm_type()).
                   op(pre_params.op());

        req_params.cmd_pre_push(cmd_params).
                   iv_addr(pre_params.iv()->pa()).
                   src_msg_desc_addr(msg_input_desc_pool->pre_push(input_params)).
                   dst_msg_desc_addr(msg_output_desc_pool->pre_push(output_params)).
                   key_idx0(hw_cipher_params.key_idx()).
                   status_addr(status->pa()).
                   db_addr(doorbell->pa()).
                   db_data(doorbell->data());
        req_desc.pre_push(req_params);

        msg_input_desc_pool->push();
        msg_output_desc_pool->push();
        aes_params.acc_ring()->push(req_desc.push(), aes_params.push_type(),
                                    aes_params.seq_qid());
    }
    return true;
}


/*
 * Invoked from PSE engine to cleanup after a HW cipher operation.
 */
bool
aes_t::push(aes_hw_cleanup_params_t& hw_cleanup_params)
{
    key_destroy(false);
    msg_input_vec.clear();
    msg_output_vec.clear();
    evp_cipher_ctx = nullptr;
    return true;
}

/*
 * HW trigger
 */
bool
aes_t::post_push(void)
{
    aes_params.acc_ring()->post_push();
    return true;
}


/*
 * Check status/doorbell for completion and whether there were errors.
 */
bool
aes_t::completion_check(void)
{
    bool    failure_expected = push_params.failure_expected();

    auto busy_check = [this] () -> int
    {
        if (doorbell->busy_check() || status->busy_check()) {
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
    if (poll(busy_check, failure_expected)) {
        OFFL_FUNC_ERR_OR_DEBUG(failure_expected,
                               "aes_t status is busy");
        return false;
    }
    test_success = doorbell->success_check(failure_expected) &&
                   status->success_check(failure_expected);
    return test_success;
}


/*
 * Test result full verification
 */
bool
aes_t::full_verify(void)
{
    if (!completion_check()) {
        return false;
    }

    test_success = expected_actual_verify(
                            pre_params.op() == crypto_symm::CRYPTO_SYMM_OP_ENCRYPT ?
                            "CIPHERTEXT" : "PLAINTEXT",
                            push_params.msg_output_expected(),
                            push_params.msg_output_actual());
    return test_success;
}


/*
 * Create a HW symmetric key.
 */
bool
aes_t::key_create(aes_hw_init_params_t& hw_init_params)
{
    types::CryptoKeyType    key_type;
    uint32_t                symm_key_idx;
    uint32_t                key_len;
    char                    key[CRYPTO_AES_KEY_NUM_PARTS_MAX *
                                CRYPTO_AES_KEY_PART_SIZE_MAX];
    key_len = hw_init_params.key()->content_size_get();
    switch (key_len) {

    case CRYPTO_AES_128_KEY_SIZE:
        key_type = types::CRYPTO_KEY_TYPE_AES128;
        break;
    case CRYPTO_AES_192_KEY_SIZE:
        key_type = types::CRYPTO_KEY_TYPE_AES192;
        break;
    case CRYPTO_AES_256_KEY_SIZE:
        key_type = types::CRYPTO_KEY_TYPE_AES256;
        break;
    default:
        OFFL_FUNC_ERR("invalid key length {}", key_len);
        return false;
    }

    /*
     * HW anticipates possible T10 sector usage by arranging for 2 consecutive
     * keys in its RAM, each is of a max of 32 bytes in length. The hal_if API
     * expects its argument the same way.
     */
    memset(key, 0, sizeof(key));
    memcpy(&key[0 * CRYPTO_AES_KEY_PART_SIZE_MAX],
           hw_init_params.key()->read(), key_len);
    memcpy(&key[1 * CRYPTO_AES_KEY_PART_SIZE_MAX],
           hw_init_params.key()->read(), key_len);
    if (hal_if::get_key_index(key, key_type, sizeof(key), &symm_key_idx)) {
        OFFL_FUNC_ERR("failed to create symmetric key");
        return false;
    }

    key_idx = symm_key_idx;
    OFFL_FUNC_DEBUG("created key_idx {}", key_idx);
    return true;
}

/*
 * Destroy the current symmetric key.
 */
void
aes_t::key_destroy(bool force_destroy)
{
    if ((!pre_params.key_idx_shared() || force_destroy) &&
        crypto_symm::key_idx_is_valid(key_idx)) {

        OFFL_FUNC_DEBUG("destroying key_idx {}", key_idx);
        if (hal_if::delete_key(key_idx)) {
            OFFL_FUNC_ERR("failed to destroy key_idx {}", key_idx);
        }
        key_idx = CRYPTO_SYMM_KEY_IDX_INVALID;
    }
}


/*
 * Verify expected vs. actual
 */
bool
aes_t::expected_actual_verify(const char *entity_name,
                              dp_mem_t *expected,
                              dp_mem_t *actual)
{
    /*
     * Verification is optional and is done only when 'expected' is present.
     */
    if (!expected || !expected->content_size_get()) {
        return true;
    }
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
aes_t::test_params_report(void)
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
    OFFL_LOG_DEBUG("  crypto_symm_type {}", pre_params.crypto_symm_type());
    OFFL_LOG_DEBUG("  op {}", pre_params.op() == crypto_symm::CRYPTO_SYMM_OP_ENCRYPT ?
                              "ENCRYPT" : "DECRYPT");
    dp_mem_dump("      Key", pre_params.key());
    dp_mem_dump("      IV", pre_params.iv());
    dp_mem_dump("      msg_input", push_params.msg_input());
    dp_mem_dump("      msg_output_expected", push_params.msg_output_expected());
    dp_mem_dump("      msg_output_actual", push_params.msg_output_actual());
    OFFL_LOG_DEBUG("   Result {}", push_params.failure_expected() ? "F" : "P");
}


/*
 * Access methods for PSE Openssl engine
 */
extern "C" {

static int
init(void *ctx,
     const PSE_CIPHER_INIT_PARAM *param)
{
    aes_t                   *crypto_aes = static_cast<aes_t *>(ctx);
    dp_mem_t                *key;
    aes_hw_init_params_t    init_params;

    key = static_cast<dp_mem_t *>((void *)param->key);
    return crypto_aes->push(init_params.key(key)) ? 1 : 0;
}

static int
do_cipher(void *ctx,
          const PSE_CIPHER_DO_CIPHER_PARAM *param)
{
    aes_t                   *crypto_aes = static_cast<aes_t *>(ctx);
    dp_mem_t                *msg_input;
    dp_mem_t                *msg_output;
    aes_hw_cipher_params_t  cipher_params;
    bool                    success;

    msg_input = static_cast<dp_mem_t *>((void *)param->msg_input);
    msg_output = static_cast<dp_mem_t *>((void *)param->msg_output);
    success = crypto_aes->push(cipher_params.key_idx(param->key_idx).
                                             msg_input(msg_input).
                                             msg_output(msg_output));
    /*
     * Absence of msg_input means HW execution would have 
     * been initiated above.
     */
    if (success && !msg_input) {
        if (param->wait_for_completion) {
            crypto_aes->post_push();
            success = crypto_aes->completion_check();
            if (success) {
                msg_output->read_thru();
            }
        }
    }

    return success ? 1 : 0;
}

static int
cleanup(void *ctx,
        const PSE_CIPHER_CLEANUP_PARAM *param)
{
    aes_t                   *crypto_aes = static_cast<aes_t *>(ctx);
    aes_hw_cleanup_params_t cleanup_params;

    return crypto_aes->push(cleanup_params) ? 1 : 0;
}

const PSE_OFFLOAD_CIPHER_METHOD pse_cipher_offload_method =
{
    .init       = init,
    .do_cipher  = do_cipher,
    .cleanup    = cleanup,
    .mem_method = &pse_mem_method,
};

} // extern "C"


} // namespace crypto_aes


