#include "crypto_sha.hpp"
#include "hal_if.hpp"
#include "utils.hpp"

/*
 * Secure Hash Algorithm
 */
namespace crypto_sha {

/*
 * Message descriptor (i.e., AOL) pool, enough to support a dozen
 * or so message input fragments (each AOL can hold 3 fragments).
 */
#define SHA_MSG_DESC_IDX_MAX    4

/*
 * Map SHA length to algorithm name
 */
static const map<uint32_t,string> sha_algo_map = {
    {SHA_DIGEST_LENGTH,         "SHA1"},
    {SHA224_DIGEST_LENGTH,      "SHA224"},
    {SHA256_DIGEST_LENGTH,      "SHA256"},
    {SHA384_DIGEST_LENGTH,      "SHA384"},
    {SHA512_DIGEST_LENGTH,      "SHA512"},
};

static const map<string,uint32_t> sha_nbytes_map = {
    {"SHA1",        SHA_DIGEST_LENGTH},
    {"SHA224",      SHA224_DIGEST_LENGTH},
    {"SHA256",      SHA256_DIGEST_LENGTH},
    {"SHA384",      SHA384_DIGEST_LENGTH},
    {"SHA512",      SHA512_DIGEST_LENGTH},
};

static const map<uint32_t,string> sha3_algo_map = {
    {SHA224_DIGEST_LENGTH,      "SHA3-224"},
    {SHA256_DIGEST_LENGTH,      "SHA3-256"},
    {SHA384_DIGEST_LENGTH,      "SHA3-384"},
    {SHA512_DIGEST_LENGTH,      "SHA3-512"},
};

static const map<string,uint32_t> sha3_nbytes_map = {
    {"SHA3-224",    SHA224_DIGEST_LENGTH},
    {"SHA3-256",    SHA256_DIGEST_LENGTH},
    {"SHA3-384",    SHA384_DIGEST_LENGTH},
    {"SHA3-512",    SHA512_DIGEST_LENGTH},
};

/*
 * Convert SHA length to algo name
 */
string
sha_algo_find(uint32_t sha_nbytes)
{
    auto iter = sha_algo_map.find(sha_nbytes);
    if (iter != sha_algo_map.end()) {
        return iter->second;
    }
    return "";
}

uint32_t
sha_nbytes_find(const string& sha_algo)
{
    auto iter = sha_nbytes_map.find(sha_algo);
    if (iter != sha_nbytes_map.end()) {
        return iter->second;
    }
    return 0;
}

/*
 * Convert SHA3 length to algo name
 */
string
sha3_algo_find(uint32_t sha_nbytes)
{
    auto iter = sha3_algo_map.find(sha_nbytes);
    if (iter != sha3_algo_map.end()) {
        return iter->second;
    }
    return "";
}

uint32_t
sha3_nbytes_find(const string& sha_algo)
{
    auto iter = sha3_nbytes_map.find(sha_algo);
    if (iter != sha3_nbytes_map.end()) {
        return iter->second;
    }
    return 0;
}

/*
 * Constructor
 */
sha_t::sha_t(sha_params_t& params) :
    sha_params(params),
    evp_md(nullptr),
    hw_started(false),
    test_success(true)
{
    crypto_symm::msg_desc_pool_params_t msg_params;

    assert(params.acc_ring());
    msg_desc_pool = new crypto_symm::msg_desc_pool_t(
                        msg_params.msg_desc_mem_type(params.msg_desc_mem_type()).
                                   num_descs(SHA_MSG_DESC_IDX_MAX).
                                   base_params(params.base_params()));

    status = new crypto_symm::status_t(params.status_mem_type());
    doorbell = new crypto_symm::doorbell_t(params.doorbell_mem_type());
}


/*
 * Destructor
 */
sha_t::~sha_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    sha_params.base_params().destructor_free_buffers());
    if (sha_params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (doorbell) delete doorbell;
            if (status) delete status;
            if (msg_desc_pool) delete msg_desc_pool;
        }
    }
}


/*
 * Find matching SHA algorithm
 */
bool
sha_t::pre_push(sha_pre_push_params_t& pre_params)
{
    this->pre_params = pre_params;
    hw_started = false;
    test_success = false;

    switch (pre_params.crypto_symm_type()) {

    case crypto_symm::CRYPTO_SYMM_TYPE_SHA:
        sha_algo.assign(sha_algo_find(pre_params.sha_nbytes()));
        break;

    case crypto_symm::CRYPTO_SYMM_TYPE_SHA3:
        sha_algo.assign(sha3_algo_find(pre_params.sha_nbytes()));
        break;

    default:
        break;
    }

    if (sha_algo.empty()) {
        OFFL_FUNC_ERR("failed to locate SHA algorithm for type {} size {}",
                       pre_params.crypto_symm_type(), pre_params.sha_nbytes());
        return false;
    }
    return true;
}


/*
 * Invoke engine i/f to generate a message digest.. 
 * Note engine i/f will then proceed to invoking 
 * sha_t::push() with HW params down below.
 */
bool
sha_t::push(sha_push_params_t& push_params)
{
    eng_if::digest_params_t     digest_params;

    this->push_params = push_params;
    hw_started = true;

    /*
     * Msg length of 0 is a special case which HW cannot handle,
     * in which case, we let Openssl take care of it. In other words,
     * we set sha_hw_ctx to nullptr in that case.
     */
    digest_params.sha_hw_ctx(push_params.msg()->content_size_get() ?
                             this : nullptr);
    evp_md = eng_if::digest_gen(digest_params.hash_algo(sha_algo).
                                              msg(push_params.msg()).
                                              digest(push_params.md_actual()).
                                              failure_expected(push_params.failure_expected()).
                                              wait_for_completion(push_params.wait_for_completion()));
    if (!evp_md) {
        OFFL_FUNC_ERR("failed msg_digest push");
        return false;
    }

    push_params.ret_evp_md(evp_md);
    return true;
}


/*
 * Invoked from PSE engine to commence a HW digest operation.
 */
bool
sha_t::push(sha_hw_init_params_t& hw_init_params)
{
    /*
     * HW automatically loads the SHA seed so nothing to do. 
     */
    return true;
}

/*
 * Invoked from PSE engine to incrementally add a msg_input.
 */
bool
sha_t::push(sha_hw_update_params_t& hw_update_params)
{
    msg_input_vec.push_back(hw_update_params.msg_input());
    return true;
}

/*
 * Invoked from PSE engine to calculate hash digest for
 * all the msg_input's collected thus far.
 */
bool
sha_t::push(sha_hw_final_params_t& hw_final_params)
{
    crypto_symm::req_desc_t                 req_desc;
    crypto_symm::cmd_pre_push_params_t      cmd_params;
    crypto_symm::req_desc_pre_push_params_t req_params;
    crypto_symm::msg_desc_pool_pre_push_params_t 
                                            msg_params(msg_input_vec);
    /*
     * Initialize status for later polling
     */
    status->init();
    doorbell->init();

    cmd_params.type(pre_params.crypto_symm_type()).
               op(crypto_symm::CRYPTO_SYMM_OP_GENERATE).
               sha_nbytes(pre_params.sha_nbytes());

    req_params.cmd_pre_push(cmd_params).
               src_msg_desc_addr(msg_desc_pool->pre_push(msg_params)).
               auth_tag_addr(hw_final_params.hash_output()->pa()).
               status_addr(status->pa()).
               db_addr(doorbell->pa()).
               db_data(doorbell->data());
    req_desc.pre_push(req_params);

    msg_desc_pool->push();
    sha_params.acc_ring()->push(req_desc.push(), sha_params.push_type(),
                                sha_params.seq_qid());
    return true;
}

/*
 * Invoked from PSE engine to cleanup after a HW digest operation.
 */
bool
sha_t::push(sha_hw_cleanup_params_t& hw_cleanup_params)
{
    msg_input_vec.clear();
    return true;
}

/*
 * HW trigger
 */
bool
sha_t::post_push(void)
{
    sha_params.acc_ring()->post_push();
    return true;
}


/*
 * Check status/doorbell for completion and whether there were errors.
 */
bool
sha_t::completion_check(void)
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

    /*
     * Msg length of 0 is a special case where we passed it
     * to Openssl but not HW.
     */
    if (!push_params.msg()->content_size_get()) {
        test_success = true;
    } else {
        test_success = false;
        if (poll(busy_check, failure_expected)) {
            OFFL_FUNC_ERR_OR_DEBUG(failure_expected,
                                   "sha_t status is busy");
            return false;
        }
        test_success = doorbell->success_check(failure_expected) &&
                       status->success_check(failure_expected);
    }
    return test_success;
}


/*
 * Test result full verification
 */
bool
sha_t::full_verify(void)
{
    if (!completion_check()) {
        return false;
    }

    test_success = expected_actual_verify("MD",
                                          push_params.md_expected(),
                                          push_params.md_actual());
    return test_success;
}


/*
 * Verify expected vs. actual
 */
bool
sha_t::expected_actual_verify(const char *entity_name,
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
sha_t::test_params_report(void)
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
    OFFL_LOG_DEBUG("  SHAalgo {}", sha_algo);
    dp_mem_dump("      Msg", push_params.msg());
    OFFL_LOG_DEBUG("   Result {}", push_params.failure_expected() ? "F" : "P");
}


/*
 * Access methods for PSE Openssl engine
 */
extern "C" {

static int
init(void *ctx,
     const PSE_MD_INIT_PARAM *param)
{
    sha_t                   *crypto_sha = static_cast<sha_t *>(ctx);
    sha_hw_init_params_t    init_params;

    return crypto_sha->push(init_params) ? 1 : 0;
}

static int
update(void *ctx,
       const PSE_MD_UPDATE_PARAM *param)
{
    sha_t                   *crypto_sha = static_cast<sha_t *>(ctx);
    dp_mem_t                *msg_input;
    sha_hw_update_params_t  update_params;

    msg_input = static_cast<dp_mem_t *>((void *)param->msg_input);
    return crypto_sha->push(update_params.msg_input(msg_input)) ? 1 : 0;
}

static int
final(void *ctx,
      const PSE_MD_FINAL_PARAM *param)
{
    sha_t                   *crypto_sha = static_cast<sha_t *>(ctx);
    dp_mem_t                *hash_output;
    sha_hw_final_params_t   final_params;
    bool                    success;

    hash_output = static_cast<dp_mem_t *>((void *)param->hash_output);
    success = crypto_sha->push(final_params.hash_output(hash_output));
    if (success) {
        if (param->wait_for_completion) {
            crypto_sha->post_push();
            success = crypto_sha->completion_check();
            if (success) {
                hash_output->read_thru();
            }
        }
    }

    return success ? 1 : 0;
}

static int
cleanup(void *ctx,
        const PSE_MD_CLEANUP_PARAM *param)
{
    sha_t                   *crypto_sha = static_cast<sha_t *>(ctx);
    sha_hw_cleanup_params_t cleanup_params;

    return crypto_sha->push(cleanup_params) ? 1 : 0;
}

const PSE_OFFLOAD_MD_METHOD     pse_md_offload_method =
{
    .init       = init,
    .update     = update,
    .final      = final,
    .cleanup    = cleanup,
    .mem_method = &pse_mem_method,
};

} // extern "C"


} // namespace crypto_sha


