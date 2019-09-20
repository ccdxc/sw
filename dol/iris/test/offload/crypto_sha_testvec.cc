#include "crypto_sha_testvec.hpp"
#include "utils.hpp"

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) prior to parsing.
 */
#define PARSE_TOKEN_STR_SHA_LEN         "L"
#define PARSE_TOKEN_STR_SEED            "Seed"
#define PARSE_TOKEN_STR_MSG_LEN         "Len"
#define PARSE_TOKEN_STR_MSG             "Msg"
#define PARSE_TOKEN_STR_MD              "MD"
#define PARSE_TOKEN_STR_COUNT           "COUNT"

/*
 * Unstripped versions
 */
#define PARSE_STR_SHA_LEN_PREFIX        "[L = "
#define PARSE_STR_SHA_LEN_SUFFIX        "]\n"
#define PARSE_STR_SEED_PREFIX           "Seed = "
#define PARSE_STR_SEED_SUFFIX           "\n"
#define PARSE_STR_MSG_LEN_PREFIX        "Len = "
#define PARSE_STR_MSG_PREFIX            "Msg = "
#define PARSE_STR_MD_PREFIX             "MD = "
#define PARSE_STR_MD_SUFFIX             "\n"
#define PARSE_STR_COUNT_PREFIX          "COUNT = "

namespace crypto_sha {

#define FOR_EACH_TEST_REPR(test_repr)                                    \
    for (uint32_t k = 0; k < test_repr_vec.size(); k++) {                \
        auto test_repr = test_repr_vec.at(k);

#define END_FOR_EACH_TEST_REPR(test_repr)                                \
    }

#define FOR_EACH_MSG_REPR(test_repr, msg_repr)                           \
    for (uint32_t m = 0; m < test_repr->msg_repr_vec.size(); m++) {      \
        auto msg_repr = test_repr->msg_repr_vec.at(m);
        
#define END_FOR_EACH_MSG_REPR(test_repr, msg_repr)                       \
    }


enum {
    PARSE_TOKEN_ID_SHA_LEN      = PARSE_TOKEN_ID_USER,
    PARSE_TOKEN_ID_SEED,
    PARSE_TOKEN_ID_MSG_LEN,
    PARSE_TOKEN_ID_MSG,
    PARSE_TOKEN_ID_COUNT,
    PARSE_TOKEN_ID_MD,
};

const static map<string,parser_token_id_t>      token2id_map =
{
    {PARSE_TOKEN_STR_SHA_LEN,   PARSE_TOKEN_ID_SHA_LEN},
    {PARSE_TOKEN_STR_SEED,      PARSE_TOKEN_ID_SEED},
    {PARSE_TOKEN_STR_MSG_LEN,   PARSE_TOKEN_ID_MSG_LEN},
    {PARSE_TOKEN_STR_MSG,       PARSE_TOKEN_ID_MSG},
    {PARSE_TOKEN_STR_COUNT,     PARSE_TOKEN_ID_COUNT},
    {PARSE_TOKEN_STR_MD,        PARSE_TOKEN_ID_MD},
};

/*
 * Constructor
 */
sha_testvec_t::sha_testvec_t(sha_testvec_params_t& params) :

    testvec_params(params),
    testvec_parser(nullptr),
    rsp_output(nullptr),
    num_test_failures(0),
    hw_started(false),
    test_success(false)
{
}


/*
 * Destructor
 */
sha_testvec_t::~sha_testvec_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success,
                    testvec_params.base_params().destructor_free_buffers());
    if (testvec_params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (rsp_output) delete rsp_output;
            if (testvec_parser) delete testvec_parser;
        }
    }
}


/*
 * Execute any extra pre-push initialization
 */
bool
sha_testvec_t::pre_push(sha_testvec_pre_push_params_t& pre_params)
{
    shared_ptr<sha_test_repr_t> test_repr;
    shared_ptr<sha_msg_repr_t>  msg_repr;
    testvec_parse_params_t      params;
    parser_token_id_t           token_id;
    token_parser_t              token_parser;

    OFFL_FUNC_INFO("test vector file {}", pre_params.testvec_fname());
    this->pre_params = pre_params;
    hw_started = false;
    test_success = false;

    rsp_output = new testvec_output_t(pre_params.scripts_dir(),
                                      pre_params.testvec_fname(),
                                      pre_params.rsp_fname_suffix());
#ifdef __x86_64__

    /*
     * Pensando FIPS consulting cannot parse product info so
     * leave it out for real HW.
     */
    rsp_output->text_vec("# ", product_info_vec_get());
#endif

    /*
     * For ease of parsing, consider brackets as whitespaces;
     * and "=" as token delimiter
     */
    token_parser.extra_whitespaces_add("[]");
    token_parser.extra_delims_add("=");
    testvec_parser = new testvec_parser_t(pre_params.scripts_dir(),
                                          pre_params.testvec_fname(),
                                          token_parser, token2id_map, rsp_output);
    while (!testvec_parser->eof()) {

        token_id = testvec_parser->parse(params.skip_unknown_token(true).
                                                output_header_comments(true));
        switch (token_id) {

        case PARSE_TOKEN_ID_EOF:
        case PARSE_TOKEN_ID_SHA_LEN:

            /*
             * EOF or a new L section terminates the current key representative
             * and msg representative.
             */
            if (test_repr.use_count()) {
                if (msg_repr.use_count()) {
                    test_repr->msg_repr_vec.push_back(move(msg_repr));
                    msg_repr.reset();
                }

                OFFL_FUNC_INFO("sha_nbytes {} with {} msg representative vectors",
                               test_repr->sha_nbytes, test_repr->msg_repr_vec.size());
                test_repr_vec.push_back(move(test_repr));
                test_repr.reset();
            }
            if (token_id == PARSE_TOKEN_ID_EOF) {
                break;
            }

            test_repr = make_shared<sha_test_repr_t>(*this);
            if (!testvec_parser->parse_ulong(&test_repr->sha_nbytes)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_MSG_LEN:
        case PARSE_TOKEN_ID_SEED:

            /*
             * Each Len or Seed section begins a new msg representative
             */
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("Len or Seed found without a test representative");
                goto error;
            }
            if (msg_repr.use_count()) {
                test_repr->msg_repr_vec.push_back(move(msg_repr));
                msg_repr.reset();
            }

            msg_repr = make_shared<sha_msg_repr_t>(*this, test_repr->sha_nbytes);
            if (token_id == PARSE_TOKEN_ID_MSG_LEN) {
                if (!testvec_parser->parse_ulong(&msg_repr->msg_nbits)) {
                    msg_repr->failed_parse_token = token_id;
                }
            } else {
                if (!testvec_parser->parse_hex_bn(msg_repr->seed)) {
                    test_repr->failed_parse_token = token_id;
                }
            }
            break;

        case PARSE_TOKEN_ID_MSG:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Msg");
                goto error;
            }
            if (!msg_repr->msg_alloc()) {
                msg_repr->failed_parse_token = token_id;
                goto error;
            }

            /*
             * Don't parse msg if len is zero
             */
            if (msg_repr->msg_nbits) {
                if (!testvec_parser->parse_hex_bn(msg_repr->msg)) {
                    msg_repr->failed_parse_token = token_id;
                }
            }
            break;

        case PARSE_TOKEN_ID_MD:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place MD");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->md_expected, true)) {
                msg_repr->failed_parse_token = token_id;
                break;
            }

            if (is_montecarlo(msg_repr.get())) {
                msg_repr->montecarlo_expected.push_back(msg_repr->md_expected);
            }
            break;

        case PARSE_TOKEN_ID_COUNT:

             /*
              * Nothing useful to do with this token
              */
            break;

        default:

            /*
             * Nothing to do as we already instructed parser to skip over
             * unknown token
             */
            break;
        }
    }

    OFFL_FUNC_INFO("with {} key representative vectors", test_repr_vec.size());
    return true;

error:
    return false;
}


/*
 * Initiate the test
 */
bool 
sha_testvec_t::push(sha_testvec_push_params_t& push_params)
{
    sha_params_t                sha_params;
    sha_pre_push_params_t       sha_pre_params;
    sha_push_params_t           sha_push_params;

    this->push_params = push_params;
    if (test_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as test representative vector is empty");
        return true;
    }

    hw_started = true;
    sha_params.base_params(testvec_params.base_params()).
               msg_desc_mem_type(testvec_params.msg_desc_mem_type()).
               msg_digest_mem_type(testvec_params.msg_digest_mem_type()).
               status_mem_type(testvec_params.status_mem_type()).
               doorbell_mem_type(testvec_params.doorbell_mem_type()).
               acc_ring(push_params.sha_ring()).
               push_type(push_params.push_type()).
               seq_qid(push_params.seq_qid());

    FOR_EACH_TEST_REPR(test_repr) {
        FOR_EACH_MSG_REPR(test_repr, msg_repr) {
            msg_repr->push_failure = false;

            /*
             * If there were parse errors, skip this representative.
             */
            if ((test_repr->failed_parse_token != PARSE_TOKEN_ID_VOID) ||
                (msg_repr->failed_parse_token != PARSE_TOKEN_ID_VOID)) {

                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected, "Parser error"
                                       " on test token {} or msg token {}",
                                       test_repr->failed_parse_token,
                                       msg_repr->failed_parse_token);
                msg_repr->push_failure = true;
                continue;
            }
            if (!msg_repr->crypto_sha) {
                msg_repr->crypto_sha = new sha_t(sha_params);
            }

            sha_pre_params.crypto_symm_type(testvec_params.crypto_symm_type()).
                           sha_nbytes(msg_repr->sha_nbytes);
            if (!msg_repr->crypto_sha->pre_push(sha_pre_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_sha pre_push");
                msg_repr->push_failure = true;

            } else {

                if (is_montecarlo(msg_repr.get())) {
                    montecarlo_execute(msg_repr.get());
                    continue;
                }

                sha_push_params.msg(msg_repr->msg).
                                md_expected(msg_repr->md_expected).
                                md_actual(msg_repr->md_actual).
                                failure_expected(msg_repr->failure_expected).
                                wait_for_completion(false);
                if (!msg_repr->crypto_sha->push(sha_push_params)) {
                    OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                           "failed crypto_sha push");
                    msg_repr->push_failure = true;
                }
            }

        } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)

    } END_FOR_EACH_TEST_REPR(test_repr)

    return true;
}

/*
 * Having a seed means this is a MonteCarlo test
 */
bool
sha_testvec_t::is_montecarlo(sha_msg_repr_t *msg_repr)
{
    return msg_repr->seed->content_size_get();
}

/*
 * Set up source input message for a Montecarlo test.
 */
bool
sha_testvec_t::montecarlo_msg_set(sha_msg_repr_t *msg_repr,
                                  dp_mem_t *seed0,
                                  dp_mem_t *seed1,
                                  dp_mem_t *seed2)
{
    uint32_t    total_nbytes;
    bool        success;

    success = eng_if::dp_mem_to_dp_mem(msg_repr->msg_mct0, seed0);
    total_nbytes = msg_repr->msg_mct0->content_size_get();
    if (success && seed1) {
        success = eng_if::dp_mem_to_dp_mem(msg_repr->msg_mct1, seed1);
        total_nbytes += msg_repr->msg_mct1->content_size_get();
    }
    if (success && seed2) {
        success = eng_if::dp_mem_to_dp_mem(msg_repr->msg_mct2, seed2);
        total_nbytes += msg_repr->msg_mct2->content_size_get();
    }

    msg_repr->msg_mct_vec->content_size_set(total_nbytes);
    return success;
}

/*
 * Execute a Montecarlo test.
 */
void
sha_testvec_t::montecarlo_execute(sha_msg_repr_t *msg_repr)
{
    dp_mem_t            *seed0;
    dp_mem_t            *seed1;
    dp_mem_t            *seed2;
    sha_push_params_t   sha_push_params;
    uint32_t            iters;
    uint32_t            epoch;
    uint32_t            liveness = 0;

    if (!testvec_params.montecarlo_iters_max()          ||
        !testvec_params.montecarlo_result_epoch()       ||
        (testvec_params.montecarlo_iters_max() < 
                 testvec_params.montecarlo_result_epoch())) {
        OFFL_FUNC_ERR("invalid montecarlo_iters_max {} or result_epoch {}",
                      testvec_params.montecarlo_iters_max(),
                      testvec_params.montecarlo_result_epoch());
        msg_repr->push_failure = true;
        return;
    }

    seed0 = msg_repr->seed;
    seed1 = seed2 = nullptr;
    if (testvec_params.crypto_symm_type() == 
                       crypto_symm::CRYPTO_SYMM_TYPE_SHA) {
        seed1 = seed2 = msg_repr->seed;
    }

    iters = testvec_params.montecarlo_iters_max() /
            testvec_params.montecarlo_result_epoch();
    while (iters--) {
        for (epoch = 0; 
             epoch < testvec_params.montecarlo_result_epoch();
             epoch++) {

            if (!montecarlo_msg_set(msg_repr, seed0, seed1, seed2)) {
                OFFL_FUNC_ERR("failed montecarlo_msg_set");
                msg_repr->push_failure = true;
                return;
            }

            sha_push_params.msg(msg_repr->msg_mct_vec).
                            md_actual(msg_repr->md_actual).
                            failure_expected(msg_repr->failure_expected).
                            wait_for_completion(true);
            if (!msg_repr->crypto_sha->push(sha_push_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_sha push");
                msg_repr->push_failure = true;
                return;
            }

            msg_repr->crypto_sha->post_push();
            if (!msg_repr->crypto_sha->completion_check()) {
                msg_repr->push_failure = true;
                return;
            }

            seed0 = msg_repr->md_actual;
            if (testvec_params.crypto_symm_type() == 
                               crypto_symm::CRYPTO_SYMM_TYPE_SHA) {
                seed0 = msg_repr->msg_mct1;
                seed1 = msg_repr->msg_mct2;
                seed2 = msg_repr->md_actual;
            }
        }

        if ((++liveness % SHA_MONTECARLO_RESULT_LIVENESS) == 0) {
            OFFL_FUNC_INFO("completed epoch {}", liveness);
        }

        if (!msg_repr->montecarlo_actual.push_back(msg_repr->md_actual)) {
            OFFL_FUNC_ERR("failed montecarlo_actual push_back");
            msg_repr->push_failure = true;
            return;
        }

        if (testvec_params.crypto_symm_type() == 
                           crypto_symm::CRYPTO_SYMM_TYPE_SHA) {
            seed0 = seed1 = seed2 = msg_repr->md_actual;
        }
    }
}

/*
 *
 */
bool 
sha_testvec_t::post_push(void)
{
    push_params.sha_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool 
sha_testvec_t::completion_check(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_TEST_REPR(test_repr) {
            FOR_EACH_MSG_REPR(test_repr, msg_repr) {

                /*
                 * if push already failed, don't check completion
                 */
                msg_repr->compl_failure = false;
                if (msg_repr->push_failure) {
                    if (!msg_repr->failure_expected) {
                       num_test_failures++;
                    }
                    continue;
                }

                if (!msg_repr->crypto_sha->completion_check()) {
                    msg_repr->compl_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("sha_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Test result full verification
 */
bool 
sha_testvec_t::full_verify(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_TEST_REPR(test_repr) {
            FOR_EACH_MSG_REPR(test_repr, msg_repr) {

                /*
                 * if push already failed, don't bother with verify
                 */
                msg_repr->verify_failure = false;
                if (msg_repr->push_failure) {
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                    continue;
                }

                if (!msg_repr->crypto_sha->full_verify()) {
                    msg_repr->verify_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

                /*
                 * verify expected/actual Montecarlo results if any
                 */
                if (is_montecarlo(msg_repr.get())) {
                    for (uint32_t idx = 0; 
                         idx < msg_repr->montecarlo_actual.count();
                         idx++) {

                        if (msg_repr->montecarlo_expected.result_to_dp_mem(idx,
                                                 msg_repr->md_expected) &&
                            msg_repr->montecarlo_actual.result_to_dp_mem(idx,
                                                 msg_repr->md_actual)) {
                            if (!msg_repr->crypto_sha->expected_actual_verify(
                                                "MD", msg_repr->md_expected,
                                                msg_repr->md_actual)) {
                                msg_repr->verify_failure = true;
                                if (!msg_repr->failure_expected) {
                                    num_test_failures++;
                                }
                            }
                        } else {
                            break;
                        }
                    }
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("sha_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Generate testvector response output file
 */
void 
sha_testvec_t::rsp_file_output(void)
{
    if (hw_started && rsp_output) {
        FOR_EACH_TEST_REPR(test_repr) {

            rsp_output->dec(PARSE_STR_SHA_LEN_PREFIX, test_repr->sha_nbytes,
                            PARSE_STR_SHA_LEN_SUFFIX);
            FOR_EACH_MSG_REPR(test_repr, msg_repr) {

                if (is_montecarlo(msg_repr.get())) {
                    rsp_output->hex_bn(PARSE_STR_SEED_PREFIX, msg_repr->seed,
                                       PARSE_STR_SEED_SUFFIX);
                    for (uint32_t idx = 0; 
                         idx < msg_repr->montecarlo_actual.count();
                         idx++) {

                        rsp_output->dec(PARSE_STR_COUNT_PREFIX, idx);
                        msg_repr->montecarlo_actual.result_to_dp_mem(idx,
                                                           msg_repr->md_actual);
                        rsp_output->hex_bn(PARSE_STR_MD_PREFIX, msg_repr->md_actual,
                                           PARSE_STR_MD_SUFFIX);
                    }
                } else {
                    rsp_output->dec(PARSE_STR_MSG_LEN_PREFIX, msg_repr->msg_nbits);

                    /*
                     * FIPS requires printing at least one byte of msg (e.g., "00") 
                     */
                    if (!msg_repr->msg->content_size_get()) {
                        msg_repr->msg->content_size_set(1);
                    }
                    rsp_output->hex_bn(PARSE_STR_MSG_PREFIX, msg_repr->msg);
                    rsp_output->hex_bn(PARSE_STR_MD_PREFIX, msg_repr->md_actual,
                                       PARSE_STR_MD_SUFFIX);
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (rsp_output) {
        delete rsp_output;
        rsp_output = nullptr;
    }
}

} // namespace crypto_sha
