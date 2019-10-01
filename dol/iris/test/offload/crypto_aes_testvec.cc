#include "crypto_aes_testvec.hpp"
#include "utils.hpp"

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) prior to parsing.
 */
#define PARSE_TOKEN_STR_ENCRYPT         "ENCRYPT"
#define PARSE_TOKEN_STR_DECRYPT         "DECRYPT"
#define PARSE_TOKEN_STR_KEY             "KEY"
#define PARSE_TOKEN_STR_IV              "IV"
#define PARSE_TOKEN_STR_PT              "PLAINTEXT"
#define PARSE_TOKEN_STR_CT              "CIPHERTEXT"
#define PARSE_TOKEN_STR_COUNT           "COUNT"

/*
 * Unstripped versions
 */
#define PARSE_TOKEN_STR_ENCRYPT_PREFIX  "["
#define PARSE_TOKEN_STR_ENCRYPT_SUFFIX  "]\n"
#define PARSE_TOKEN_STR_DECRYPT_PREFIX  "["
#define PARSE_TOKEN_STR_DECRYPT_SUFFIX  "]\n"
#define PARSE_STR_KEY_PREFIX            "KEY = "
#define PARSE_STR_IV_PREFIX             "IV = "
#define PARSE_TOKEN_STR_PT_PREFIX       "PLAINTEXT = "
#define PARSE_TOKEN_STR_PT_SUFFIX       "\n"
#define PARSE_TOKEN_STR_CT_PREFIX       "CIPHERTEXT = "
#define PARSE_TOKEN_STR_CT_SUFFIX       "\n"
#define PARSE_STR_COUNT_PREFIX          "COUNT = "

namespace crypto_aes {

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
    PARSE_TOKEN_ID_ENCRYPT      = PARSE_TOKEN_ID_USER,
    PARSE_TOKEN_ID_DECRYPT,
    PARSE_TOKEN_ID_KEY,
    PARSE_TOKEN_ID_IV,
    PARSE_TOKEN_ID_COUNT,
    PARSE_TOKEN_ID_PT,
    PARSE_TOKEN_ID_CT,
};

const static map<string,parser_token_id_t>      token2id_map =
{
    {PARSE_TOKEN_STR_ENCRYPT,   PARSE_TOKEN_ID_ENCRYPT},
    {PARSE_TOKEN_STR_DECRYPT,   PARSE_TOKEN_ID_DECRYPT},
    {PARSE_TOKEN_STR_KEY,       PARSE_TOKEN_ID_KEY},
    {PARSE_TOKEN_STR_IV,        PARSE_TOKEN_ID_IV},
    {PARSE_TOKEN_STR_COUNT,     PARSE_TOKEN_ID_COUNT},
    {PARSE_TOKEN_STR_PT,        PARSE_TOKEN_ID_PT},
    {PARSE_TOKEN_STR_CT,        PARSE_TOKEN_ID_CT},
};

/*
 * Constructor
 */
aes_testvec_t::aes_testvec_t(aes_testvec_params_t& params) :

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
aes_testvec_t::~aes_testvec_t()
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
aes_testvec_t::pre_push(aes_testvec_pre_push_params_t& pre_params)
{
    shared_ptr<aes_test_repr_t> test_repr;
    shared_ptr<aes_msg_repr_t>  msg_repr;
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
        case PARSE_TOKEN_ID_ENCRYPT:
        case PARSE_TOKEN_ID_DECRYPT:

            /*
             * EOF or a new direction section terminates the current test representative
             * and msg representative.
             */
            if (test_repr.use_count()) {
                if (msg_repr.use_count()) {
                    test_repr->msg_repr_vec.push_back(move(msg_repr));
                    msg_repr.reset();
                }

                OFFL_FUNC_INFO("with {} msg representative vectors",
                               test_repr->msg_repr_vec.size());
                test_repr_vec.push_back(move(test_repr));
                test_repr.reset();
            }
            if (token_id == PARSE_TOKEN_ID_EOF) {
                break;
            }

            test_repr = make_shared<aes_test_repr_t>(*this);
            test_repr->op = token_id == PARSE_TOKEN_ID_ENCRYPT ?
                            crypto_symm::CRYPTO_SYMM_OP_ENCRYPT :
                            crypto_symm::CRYPTO_SYMM_OP_DECRYPT;
            break;

        case PARSE_TOKEN_ID_COUNT:

            /*
             * Each Count section begins a new msg representative except for
             * a Montecarlo test where each count is just a sampled epoch.
             */
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("{} found without a test representative",
                              PARSE_TOKEN_STR_COUNT);
                goto error;
            }
            if (!is_montecarlo()) {
                if (msg_repr.use_count()) {
                    test_repr->msg_repr_vec.push_back(move(msg_repr));
                    msg_repr.reset();
                }
            }
            if (!msg_repr.get()) {
                msg_repr = make_shared<aes_msg_repr_t>(*this);
            }
            testvec_parser->line_consume_set();
            break;

        case PARSE_TOKEN_ID_KEY:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_KEY);
                goto error;
            }
            if (is_montecarlo()) {
                if (!testvec_parser->parse_hex_bn(msg_repr->msg_work)) {
                    msg_repr->failed_parse_token = token_id;
                }
                msg_repr->mc_key_expected.push_back(msg_repr->msg_work);
                if (msg_repr->mc_key_expected.count() == 1) {
                    if (!eng_if::dp_mem_to_dp_mem(msg_repr->key,
                                                  msg_repr->msg_work)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                }
            } else {
                if (!testvec_parser->parse_hex_bn(msg_repr->key)) {
                    msg_repr->failed_parse_token = token_id;
                }
            }
            break;

        case PARSE_TOKEN_ID_IV:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_IV);
                goto error;
            }

            if (is_montecarlo()) {
                if (!testvec_parser->parse_hex_bn(msg_repr->msg_work)) {
                    msg_repr->failed_parse_token = token_id;
                }
                msg_repr->mc_iv_expected.push_back(msg_repr->msg_work);
                if (msg_repr->mc_iv_expected.count() == 1) {
                    if (!eng_if::dp_mem_to_dp_mem(msg_repr->iv,
                                                  msg_repr->msg_work)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                }
            } else {
                if (!testvec_parser->parse_hex_bn(msg_repr->iv)) {
                    msg_repr->failed_parse_token = token_id;
                }
            }
            break;

        case PARSE_TOKEN_ID_PT:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_PT);
                goto error;
            }
            if (test_repr->is_encrypt()) {
                if (is_montecarlo()) {
                    if (!testvec_parser->parse_hex_bn(msg_repr->msg_work)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                    msg_repr->mc_input_expected.push_back(msg_repr->msg_work);
                    if (msg_repr->mc_input_expected.count() == 1) {
                        if (!eng_if::dp_mem_to_dp_mem(msg_repr->msg_input,
                                                      msg_repr->msg_work)) {
                            msg_repr->failed_parse_token = token_id;
                        }
                    }
                } else {
                    if (!testvec_parser->parse_hex_bn(msg_repr->msg_input)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                }

                /*
                 * Symmetric cipher expects msg_output buffer to be of the
                 * same length as input.
                 */
                msg_repr->msg_output_actual->content_size_set(
                          msg_repr->msg_input->content_size_get());

            } else {
                if (!testvec_parser->parse_hex_bn(msg_repr->msg_output_expected)) {
                    msg_repr->failed_parse_token = token_id;
                }
                if (is_montecarlo()) {
                    msg_repr->mc_output_expected.push_back(
                                        msg_repr->msg_output_expected);
                }
            }
            break;

        case PARSE_TOKEN_ID_CT:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_CT);
                goto error;
            }
            if (test_repr->is_encrypt()) {
                if (!testvec_parser->parse_hex_bn(msg_repr->msg_output_expected)) {
                    msg_repr->failed_parse_token = token_id;
                }
                if (is_montecarlo()) {
                    msg_repr->mc_output_expected.push_back(
                                        msg_repr->msg_output_expected);
                }

            } else {

                if (is_montecarlo()) {
                    if (!testvec_parser->parse_hex_bn(msg_repr->msg_work)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                    msg_repr->mc_input_expected.push_back(msg_repr->msg_work);
                    if (msg_repr->mc_input_expected.count() == 1) {
                        if (!eng_if::dp_mem_to_dp_mem(msg_repr->msg_input,
                                                      msg_repr->msg_work)) {
                            msg_repr->failed_parse_token = token_id;
                        }
                    }
                } else {
                    if (!testvec_parser->parse_hex_bn(msg_repr->msg_input)) {
                        msg_repr->failed_parse_token = token_id;
                    }
                }

                /*
                 * Symmetric cipher expects msg_output buffer to be of the
                 * same length as input.
                 */
                msg_repr->msg_output_actual->content_size_set(
                          msg_repr->msg_input->content_size_get());
            }
            break;

        default:

            /*
             * Nothing to do as we already instructed parser to skip over
             * unknown token
             */
            break;
        }
    }

    OFFL_FUNC_INFO("with {} test representative vectors", test_repr_vec.size());
    return true;

error:
    return false;
}


/*
 * Initiate the test
 */
bool 
aes_testvec_t::push(aes_testvec_push_params_t& push_params)
{
    aes_params_t                aes_params;
    aes_pre_push_params_t       aes_pre_params;
    aes_push_params_t           aes_push_params;

    this->push_params = push_params;
    if (test_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as test representative vector is empty");
        return true;
    }

    hw_started = true;
    aes_params.base_params(testvec_params.base_params()).
               msg_desc_mem_type(testvec_params.msg_desc_mem_type()).
               status_mem_type(testvec_params.status_mem_type()).
               doorbell_mem_type(testvec_params.doorbell_mem_type()).
               acc_ring(push_params.aes_ring()).
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
            if (!msg_repr->crypto_aes) {
                msg_repr->crypto_aes = new aes_t(aes_params);
            }

            switch (testvec_params.montecarlo_type()) {

            case crypto_aes::CRYPTO_AES_MONTECARLO_CBC:
                montecarlo_cbc_execute(test_repr.get(), msg_repr.get());
                break;

            case crypto_aes::CRYPTO_AES_NOT_MONTECARLO:
                aes_pre_params.crypto_symm_type(testvec_params.crypto_symm_type()).
                               op(test_repr->op).
                               key(msg_repr->key).
                               iv(msg_repr->iv);
                if (!msg_repr->crypto_aes->pre_push(aes_pre_params)) {
                    OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                           "failed crypto_aes pre_push");
                    msg_repr->push_failure = true;
                    break;
                }
                aes_push_params.msg_input(msg_repr->msg_input).
                                msg_output_actual(msg_repr->msg_output_actual).
                                msg_output_expected(msg_repr->msg_output_expected).
                                failure_expected(msg_repr->failure_expected).
                                wait_for_completion(false);
                if (!msg_repr->crypto_aes->push(aes_push_params)) {
                    OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                           "failed crypto_aes push");
                    msg_repr->push_failure = true;
                }
                break;

            default:
                OFFL_FUNC_ERR("unsupported Montecarlo type {}",
                              testvec_params.montecarlo_type());
                msg_repr->push_failure = true;
                break;
            }

        } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)

    } END_FOR_EACH_TEST_REPR(test_repr)

    return true;
}

/*
 * Having a seed means this is a MonteCarlo test
 */
bool
aes_testvec_t::is_montecarlo(void)
{
    return testvec_params.montecarlo_type() != 
                   crypto_aes::CRYPTO_AES_NOT_MONTECARLO;
}

/*
 * Execute a Montecarlo CBC (cipher block chaining) test.
 */
void
aes_testvec_t::montecarlo_cbc_execute(aes_test_repr_t *test_repr,
                                      aes_msg_repr_t *msg_repr)
{
    aes_pre_push_params_t aes_pre_params;
    aes_push_params_t   aes_push_params;
    uint32_t            iters;
    uint32_t            epoch;
    uint32_t            key_len;
    uint32_t            liveness = 0;
    bool                ret;

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

    aes_pre_params.crypto_symm_type(testvec_params.crypto_symm_type()).
                   op(test_repr->op).
                   key(msg_repr->key).
                   iv(msg_repr->iv);
    iters = testvec_params.montecarlo_iters_max() /
            testvec_params.montecarlo_result_epoch();
    while (iters--) {

        aes_pre_params.key_idx(CRYPTO_SYMM_KEY_IDX_INVALID).
                       key_idx_shared(true);
        if (!msg_repr->mc_key_actual.push_back(msg_repr->key)   ||
            !msg_repr->mc_iv_actual.push_back(msg_repr->iv)     ||
            !msg_repr->mc_input_actual.push_back(msg_repr->msg_input)) {
            OFFL_FUNC_ERR("failed montecarlo key/iv/input push_back");
            msg_repr->push_failure = true;
            return;
        }
        for (epoch = 0; 
             epoch < testvec_params.montecarlo_result_epoch();
             epoch++) {

            if (!msg_repr->crypto_aes->pre_push(aes_pre_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_aes pre_push");
                msg_repr->push_failure = true;
                return;
            }

            aes_push_params.msg_input(msg_repr->msg_input).
                            msg_output_actual(msg_repr->msg_output_actual).
                            failure_expected(msg_repr->failure_expected).
                            wait_for_completion(true);
            if (!msg_repr->crypto_aes->push(aes_push_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_aes push");
                msg_repr->push_failure = true;
                return;
            }

            msg_repr->crypto_aes->post_push();
            if (!msg_repr->crypto_aes->completion_check()) {
                msg_repr->push_failure = true;
                return;
            }

            ret = true;
            if (!test_repr->is_encrypt()) {
                ret &= eng_if::dp_mem_to_dp_mem(msg_repr->msg_input_tmp,
                                                msg_repr->msg_input);
            }
            if (epoch == 0) {
                ret &= eng_if::dp_mem_to_dp_mem(msg_repr->msg_input,
                                                msg_repr->iv);
            } else {
                ret &= msg_repr->mc_output_epoch.result_to_dp_mem(epoch - 1,
                                                           msg_repr->msg_input);
            }

            /*
             * AESVS contains this misleading statement: "The IV is only used
             * on the first cipher function per text string.", which implies
             * that the IV should be cleared for subsequent strings.
             *
             * The correct interpretation should be: for the subsequent strings,
             * let the AES-CBC algorithm continue to use its auto-advancing IV
             * state, which for CBC means each subsequent string should use
             * the cipher or plain text of the previous result as the IV.
             */
            if (test_repr->is_encrypt()) {
                ret &= eng_if::dp_mem_to_dp_mem(msg_repr->iv,
                                                msg_repr->msg_output_actual);
            } else {
                ret &= eng_if::dp_mem_to_dp_mem(msg_repr->iv,
                                                msg_repr->msg_input_tmp);
            }
            if (!ret) {
                OFFL_FUNC_ERR("failed inner epoch adjustment");
                msg_repr->push_failure = true;
                return;
            }

            if (!msg_repr->mc_output_epoch.push_back(msg_repr->msg_output_actual)) {
                OFFL_FUNC_ERR("failed montecarlo epoch push_back");
                msg_repr->push_failure = true;
                return;
            }

            /*
             * Share the key within this inner loop.
             */
            aes_pre_params.key_idx(msg_repr->crypto_aes->key_idx_get());
        }

        if (!msg_repr->mc_output_actual.push_back(msg_repr->msg_output_actual)) {
            OFFL_FUNC_ERR("failed montecarlo output push_back");
            msg_repr->push_failure = true;
            return;
        }

        epoch--;
        key_len = msg_repr->key->content_size_get();
        ret = true;
        switch (key_len) {

        case CRYPTO_AES_128_KEY_SIZE:
            ret &= eng_if::dp_mem_xor_dp_mem(msg_repr->key,
                                             msg_repr->msg_output_actual);
            break;

        case CRYPTO_AES_192_KEY_SIZE:
        case CRYPTO_AES_256_KEY_SIZE:
            ret &= msg_repr->mc_output_epoch.result_to_dp_mem(epoch - 1,
                                                              msg_repr->msg_work);
            ret &= eng_if::dp_mem_cat_dp_mem(msg_repr->msg_work,
                                             msg_repr->msg_output_actual);
            if (key_len == CRYPTO_AES_192_KEY_SIZE) {

                /*
                 * work with only the last 64-bits of msg_work
                 */
                ret &= eng_if::dp_mem_to_dp_mem(msg_repr->msg_work,
                                  msg_repr->msg_work,
                                  msg_repr->msg_work->content_size_get() -
                                      sizeof(uint64_t));
            }
            ret &= eng_if::dp_mem_xor_dp_mem(msg_repr->key, msg_repr->msg_work);
            break;

        default:
            OFFL_FUNC_ERR("invalid key length {}", key_len);
            msg_repr->push_failure = true;
            return;
        }

        ret &= eng_if::dp_mem_to_dp_mem(msg_repr->iv,
                                        msg_repr->msg_output_actual);
        ret &= msg_repr->mc_output_epoch.result_to_dp_mem(epoch - 1,
                                                          msg_repr->msg_input);
        if (!ret) {
            OFFL_FUNC_ERR("failed outer epoch adjustment");
            msg_repr->push_failure = true;
            return;
        }

        msg_repr->mc_output_epoch.clear();
        msg_repr->crypto_aes->key_destroy(true);

        if ((++liveness % AES_MONTECARLO_RESULT_LIVENESS) == 0) {
            OFFL_FUNC_INFO("completed epoch {}", liveness);
        }
    }
}

/*
 *
 */
bool 
aes_testvec_t::post_push(void)
{
    push_params.aes_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool 
aes_testvec_t::completion_check(void)
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

                if (!msg_repr->crypto_aes->completion_check()) {
                    msg_repr->compl_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("aes_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Test result full verification
 */
bool 
aes_testvec_t::full_verify(void)
{
    const char  *input_name;
    const char  *output_name;

    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_TEST_REPR(test_repr) {

            if (test_repr->is_encrypt()) {
                input_name = PARSE_TOKEN_STR_PT;
                output_name = PARSE_TOKEN_STR_CT;
            } else {
                input_name = PARSE_TOKEN_STR_CT;
                output_name = PARSE_TOKEN_STR_PT;
            }
                        
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

                if (!msg_repr->crypto_aes->full_verify()) {
                    msg_repr->verify_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

                /*
                 * verify expected/actual Montecarlo results, if any
                 */
                if (!is_montecarlo()) {
                    continue;
                }
                for (uint32_t idx = 0; 
                     idx < msg_repr->mc_output_actual.count();
                     idx++) {

                    /*
                     * stop verification early if output not available
                     */
                    if (!msg_repr->mc_output_expected.result_to_dp_mem(idx,
                                             msg_repr->msg_output_expected) ||
                        !msg_repr->mc_output_actual.result_to_dp_mem(idx,
                                             msg_repr->msg_output_actual)) {
                        break;
                    }
                    if (!msg_repr->crypto_aes->expected_actual_verify(
                                   output_name, msg_repr->msg_output_expected,
                                   msg_repr->msg_output_actual)) {
                        msg_repr->verify_failure = true;
                    }

                    /*
                     * verify the rest opportunistically
                     */
                    if (msg_repr->mc_key_expected.result_to_dp_mem(idx,
                                                  msg_repr->msg_work) &&
                        msg_repr->mc_key_actual.result_to_dp_mem(idx,
                                                msg_repr->key)) {
                        if (!msg_repr->crypto_aes->expected_actual_verify(
                             PARSE_TOKEN_STR_KEY, msg_repr->msg_work, msg_repr->key)) {
                            msg_repr->verify_failure = true;
                        }
                    }
                    if (msg_repr->mc_iv_expected.result_to_dp_mem(idx,
                                                 msg_repr->msg_work) &&
                        msg_repr->mc_iv_actual.result_to_dp_mem(idx,
                                               msg_repr->iv)) {
                        if (!msg_repr->crypto_aes->expected_actual_verify(
                             PARSE_TOKEN_STR_IV, msg_repr->msg_work, msg_repr->iv)) {
                            msg_repr->verify_failure = true;
                        }
                    }
                    if (msg_repr->mc_input_expected.result_to_dp_mem(idx,
                                                    msg_repr->msg_work) &&
                        msg_repr->mc_input_actual.result_to_dp_mem(idx,
                                                  msg_repr->msg_input)) {
                        if (!msg_repr->crypto_aes->expected_actual_verify(
                             input_name, msg_repr->msg_work, msg_repr->msg_input)) {
                            msg_repr->verify_failure = true;
                        }
                    }
                }
                if (msg_repr->verify_failure && !msg_repr->failure_expected) {
                    num_test_failures++;
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("aes_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Generate testvector response output file
 */
void 
aes_testvec_t::rsp_file_output(void)
{
    uint32_t    count;

    auto output_one = [this] (aes_test_repr_t *test_repr,
                              uint32_t idx,
                              dp_mem_t *key,
                              dp_mem_t *iv,
                              dp_mem_t *msg_input,
                              dp_mem_t *msg_output) -> void
    {
        rsp_output->dec(PARSE_STR_COUNT_PREFIX, idx);
        rsp_output->hex_bn(PARSE_STR_KEY_PREFIX, key);
        rsp_output->hex_bn(PARSE_STR_IV_PREFIX, iv);
        if (test_repr->is_encrypt()) {
            rsp_output->hex_bn(PARSE_TOKEN_STR_PT_PREFIX, msg_input);
            rsp_output->hex_bn(PARSE_TOKEN_STR_CT_PREFIX, msg_output,
                               PARSE_TOKEN_STR_CT_SUFFIX);
        } else {
            rsp_output->hex_bn(PARSE_TOKEN_STR_CT_PREFIX, msg_input);
            rsp_output->hex_bn(PARSE_TOKEN_STR_PT_PREFIX, msg_output,
                               PARSE_TOKEN_STR_PT_SUFFIX);
        }
    };

    if (hw_started && rsp_output) {
        FOR_EACH_TEST_REPR(test_repr) {

            count = 0;
            if (test_repr->is_encrypt()) {
                rsp_output->str(PARSE_TOKEN_STR_ENCRYPT_PREFIX,
                                PARSE_TOKEN_STR_ENCRYPT,
                                PARSE_TOKEN_STR_ENCRYPT_SUFFIX);
            } else {
                rsp_output->str(PARSE_TOKEN_STR_DECRYPT_PREFIX,
                                PARSE_TOKEN_STR_DECRYPT,
                                PARSE_TOKEN_STR_DECRYPT_SUFFIX);
            }
            FOR_EACH_MSG_REPR(test_repr, msg_repr) {

                if (is_montecarlo()) {
                    for (uint32_t idx = 0; 
                         idx < msg_repr->mc_output_actual.count();
                         idx++) {

                        int ret = true;
                        ret &= msg_repr->mc_key_actual.result_to_dp_mem(idx,
                                                   msg_repr->key);
                        ret &= msg_repr->mc_iv_actual.result_to_dp_mem(idx,
                                                   msg_repr->iv);
                        ret &= msg_repr->mc_input_actual.result_to_dp_mem(idx,
                                                   msg_repr->msg_input);
                        ret &= msg_repr->mc_output_actual.result_to_dp_mem(idx,
                                                   msg_repr->msg_output_actual);
                        if (ret) {
                            output_one(test_repr.get(), idx, msg_repr->key,
                                       msg_repr->iv, msg_repr->msg_input,
                                       msg_repr->msg_output_actual);
                        }
                    }
                } else {
                    output_one(test_repr.get(), count++, msg_repr->key,
                               msg_repr->iv, msg_repr->msg_input,
                               msg_repr->msg_output_actual);
                }

            } END_FOR_EACH_MSG_REPR(test_repr, msg_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (rsp_output) {
        delete rsp_output;
        rsp_output = nullptr;
    }
}

} // namespace crypto_aes
