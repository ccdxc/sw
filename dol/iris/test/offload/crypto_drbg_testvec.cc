#include "crypto_drbg_testvec.hpp"
#include "testvec_output.hpp"
#include "utils.hpp"

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) prior to parsing.
 */
#define PARSE_TOKEN_STR_SHA             "SHA"
#define PARSE_TOKEN_STR_COUNT           "COUNT"
#define PARSE_TOKEN_STR_PREDICT         "PredictionResistance"
#define PARSE_TOKEN_STR_ENTROPY_LEN     "EntropyInputLen"
#define PARSE_TOKEN_STR_NONCE_LEN       "NonceLen"
#define PARSE_TOKEN_STR_PSNL_STR_LEN    "PersonalizationStringLen"
#define PARSE_TOKEN_STR_ADD_INPUT_LEN   "AdditionalInputLen"
#define PARSE_TOKEN_STR_RET_BITS        "ReturnedBits"
#define PARSE_TOKEN_STR_RET_BITS_LEN    "ReturnedBitsLen"
#define PARSE_TOKEN_STR_ENTROPY         "EntropyInput"
#define PARSE_TOKEN_STR_ENTROPY_RESEED  "EntropyInputReseed"
#define PARSE_TOKEN_STR_NONCE           "Nonce"
#define PARSE_TOKEN_STR_PSNL_STR        "PersonalizationString"
#define PARSE_TOKEN_STR_ADD_INPUT       "AdditionalInput"
#define PARSE_TOKEN_STR_ADD_INPUT_RESEED "AdditionalInputReseed"
#define PARSE_TOKEN_STR_ENTROPY_PR      "EntropyInputPR"

/*
 * Unstripped versions
 */
#define PARSE_STR_SHA_PREFIX            "[SHA-"
#define PARSE_STR_SHA_SUFFIX            "]"
#define PARSE_STR_PREDICT_PREFIX        "[PredictionResistance = "
#define PARSE_STR_PREDICT_SUFFIX        "]"
#define PARSE_STR_ENTROPY_LEN_PREFIX    "[EntropyInputLen = "
#define PARSE_STR_ENTROPY_LEN_SUFFIX    "]"
#define PARSE_STR_NONCE_LEN_PREFIX      "[NonceLen = "
#define PARSE_STR_NONCE_LEN_SUFFIX      "]"
#define PARSE_STR_PSNL_STR_LEN_PREFIX   "[PersonalizationStringLen = "
#define PARSE_STR_PSNL_STR_LEN_SUFFIX   "]"
#define PARSE_STR_ADD_INPUT_LEN_PREFIX  "[AdditionalInputLen = "
#define PARSE_STR_ADD_INPUT_LEN_SUFFIX  "]"
#define PARSE_STR_RET_BITS_LEN_PREFIX   "[ReturnedBitsLen = "
#define PARSE_STR_RET_BITS_LEN_SUFFIX   "]\n"

#define PARSE_STR_COUNT_PREFIX          "COUNT = "
#define PARSE_STR_ENTROPY_PREFIX        "EntropyInput = "
#define PARSE_STR_ENTROPY_RESEED_PREFIX "EntropyInputReseed = "
#define PARSE_STR_NONCE_PREFIX          "Nonce = "
#define PARSE_STR_PSNL_STR_PREFIX       "PersonalizationString = "
#define PARSE_STR_ADD_INPUT_PREFIX      "AdditionalInput = "
#define PARSE_STR_ADD_INPUT_RESEED_PREFIX "AdditionalInputReseed = "
#define PARSE_STR_ENTROPY_PR_PREFIX     "EntropyInputPR = "
#define PARSE_STR_RET_BITS_PREFIX       "ReturnedBits = "
#define PARSE_STR_RET_BITS_SUFFIX       "\n"


namespace crypto_drbg {

#define FOR_EACH_TEST_REPR(test_repr)                                    \
    for (uint32_t k = 0; k < test_repr_vec.size(); k++) {                \
        auto test_repr = test_repr_vec.at(k);

#define END_FOR_EACH_TEST_REPR(test_repr)                                \
    }

#define FOR_EACH_TRIAL_REPR(test_repr, trial_repr)                       \
    for (uint32_t m = 0; m < test_repr->trial_repr_vec.size(); m++) {    \
        auto trial_repr = test_repr->trial_repr_vec.at(m);
        
#define END_FOR_EACH_TRIAL_REPR(test_repr, trial_repr)                   \
    }


enum {
    PARSE_TOKEN_ID_SHA          = PARSE_TOKEN_ID_USER,
    PARSE_TOKEN_ID_COUNT,
    PARSE_TOKEN_ID_PREDICT,
    PARSE_TOKEN_ID_ENTROPY_LEN,
    PARSE_TOKEN_ID_NONCE_LEN,
    PARSE_TOKEN_ID_PSNL_STR_LEN,
    PARSE_TOKEN_ID_ADD_INPUT_LEN,
    PARSE_TOKEN_ID_RET_BITS_LEN,
    PARSE_TOKEN_ID_RET_BITS,
    PARSE_TOKEN_ID_ENTROPY,
    PARSE_TOKEN_ID_NONCE,
    PARSE_TOKEN_ID_PSNL_STR,
    PARSE_TOKEN_ID_ADD_INPUT,
    PARSE_TOKEN_ID_ADD_INPUT_RESEED,
    PARSE_TOKEN_ID_ENTROPY_PR,
    PARSE_TOKEN_ID_ENTROPY_RESEED,
};

const static map<string,parser_token_id_t>      token2id_map =
{
    {PARSE_TOKEN_STR_SHA,           PARSE_TOKEN_ID_SHA},
    {PARSE_TOKEN_STR_COUNT,         PARSE_TOKEN_ID_COUNT},
    {PARSE_TOKEN_STR_PREDICT,       PARSE_TOKEN_ID_PREDICT},
    {PARSE_TOKEN_STR_ENTROPY_LEN,   PARSE_TOKEN_ID_ENTROPY_LEN},
    {PARSE_TOKEN_STR_NONCE_LEN,     PARSE_TOKEN_ID_NONCE_LEN},
    {PARSE_TOKEN_STR_PSNL_STR_LEN,  PARSE_TOKEN_ID_PSNL_STR_LEN},
    {PARSE_TOKEN_STR_ADD_INPUT_LEN, PARSE_TOKEN_ID_ADD_INPUT_LEN},
    {PARSE_TOKEN_STR_RET_BITS_LEN,  PARSE_TOKEN_ID_RET_BITS_LEN},
    {PARSE_TOKEN_STR_RET_BITS,      PARSE_TOKEN_ID_RET_BITS},
    {PARSE_TOKEN_STR_ENTROPY,       PARSE_TOKEN_ID_ENTROPY},
    {PARSE_TOKEN_STR_NONCE,         PARSE_TOKEN_ID_NONCE},
    {PARSE_TOKEN_STR_PSNL_STR,      PARSE_TOKEN_ID_PSNL_STR},
    {PARSE_TOKEN_STR_ADD_INPUT,     PARSE_TOKEN_ID_ADD_INPUT},
    {PARSE_TOKEN_STR_ADD_INPUT_RESEED, PARSE_TOKEN_ID_ADD_INPUT_RESEED},
    {PARSE_TOKEN_STR_ENTROPY_PR,    PARSE_TOKEN_ID_ENTROPY_PR},
    {PARSE_TOKEN_STR_ENTROPY_RESEED,PARSE_TOKEN_ID_ENTROPY_RESEED},
};

/*
 * Constructor
 */
drbg_testvec_t::drbg_testvec_t(drbg_testvec_params_t& params) :

    testvec_params(params),
    testvec_parser(nullptr),
    rsp_output(nullptr),
    drbg(nullptr),
    num_test_failures(0),
    hw_started(false),
    test_success(false)
{
}


/*
 * Destructor
 */
drbg_testvec_t::~drbg_testvec_t()
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
drbg_testvec_t::pre_push(drbg_testvec_pre_push_params_t& pre_params)
{
    shared_ptr<drbg_test_repr_t>  test_repr;
    shared_ptr<drbg_trial_repr_t> trial_repr;
    testvec_parse_params_t      params;
    string                      predict_resist_str;
    parser_token_id_t           token_id;
    token_parser_t              token_parser;

    OFFL_FUNC_INFO("test vector file {}", pre_params.testvec_fname());
    this->pre_params = pre_params;
    hw_started = false;
    test_success = false;

    rsp_output = new testvec_output_t(pre_params.scripts_dir(),
                                      pre_params.testvec_fname(),
                                      pre_params.instance() == DRBG_INSTANCE0 ?
                                      "" : to_string(pre_params.instance()));
#ifdef __x86_64__

    /*
     * Pensando FIPS consulting cannot parse product info so
     * leave it out for real HW.
     */
    rsp_output->text_vec("# ", product_info_vec_get());
#endif

    /*
     * For ease of parsing, consider brackets as whitespaces;
     * and "-" and "=" as token delimiters.
     */
    token_parser.extra_whitespaces_add("[]");
    token_parser.extra_delims_add("-=");
    testvec_parser = new testvec_parser_t(pre_params.scripts_dir(),
                                          pre_params.testvec_fname(),
                                          token_parser, token2id_map, rsp_output);
    while (!testvec_parser->eof()) {

        token_id = testvec_parser->parse(params.skip_unknown_token(true).
                                                output_header_comments(true));
        switch (token_id) {

        case PARSE_TOKEN_ID_EOF:
        case PARSE_TOKEN_ID_SHA:

            /*
             * EOF or a new SHA section terminates the current test representative
             * and trial representative.
             */
            if (test_repr.use_count()) {
                if (trial_repr.use_count()) {
                    test_repr->trial_repr_vec.push_back(move(trial_repr));
                    trial_repr.reset();
                }

                OFFL_FUNC_INFO("with {} trial representative vectors",
                               test_repr->trial_repr_vec.size());
                test_repr_vec.push_back(move(test_repr));
                test_repr.reset();
            }
            if (token_id == PARSE_TOKEN_ID_EOF) {
                break;
            }

            test_repr = make_shared<drbg_test_repr_t>(*this);
            if (!testvec_parser->parse_string(&test_repr->sha_algo)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_PREDICT:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_PREDICT);
                goto error;
            }
            predict_resist_str.clear();
            testvec_parser->parse_string(&predict_resist_str);
            test_repr->predict_resist_flag = !predict_resist_str.empty() &&
                                             (predict_resist_str.front() == 'T');
            break;

        case PARSE_TOKEN_ID_ENTROPY_LEN:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ENTROPY_LEN);
                goto error;
            }
            if (!testvec_parser->parse_ulong(&test_repr->entropy_nbits)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_NONCE_LEN:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_NONCE_LEN);
                goto error;
            }
            if (!testvec_parser->parse_ulong(&test_repr->nonce_nbits)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_PSNL_STR_LEN:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_PSNL_STR_LEN);
                goto error;
            }
            if (!testvec_parser->parse_ulong(&test_repr->psnl_str_nbits)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_ADD_INPUT_LEN:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ADD_INPUT_LEN);
                goto error;
            }
            if (!testvec_parser->parse_ulong(&test_repr->add_input_nbits)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_RET_BITS_LEN:
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_RET_BITS_LEN);
                goto error;
            }
            if (!testvec_parser->parse_ulong(&test_repr->ret_nbits)) {
                test_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_COUNT:

            /*
             * Each Count section begins a new trial representative
             */
            if (!test_repr.use_count()) {
                OFFL_FUNC_ERR("{} found without a test representative", PARSE_TOKEN_STR_COUNT);
                goto error;
            }
            if (trial_repr.use_count()) {
                test_repr->trial_repr_vec.push_back(move(trial_repr));
                trial_repr.reset();
            }
            trial_repr = make_shared<drbg_trial_repr_t>(*this, test_repr.get());
            if (!testvec_parser->parse_ulong(&trial_repr->count)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_ENTROPY:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ENTROPY);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->entropy)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_NONCE:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_NONCE);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->nonce)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_PSNL_STR:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_PSNL_STR);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->psnl_str, true)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_ADD_INPUT:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ADD_INPUT);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->add_input_vec, true)) {
                trial_repr->failed_parse_token = token_id;
            }
            trial_repr->add_input_vec->line_advance();
            break;

        case PARSE_TOKEN_ID_ADD_INPUT_RESEED:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ADD_INPUT_RESEED);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->add_input_reseed, true)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_ENTROPY_PR:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ENTROPY_PR);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->entropy_pr_vec)) {
                trial_repr->failed_parse_token = token_id;
            }
            trial_repr->entropy_pr_vec->line_advance();
            break;

        case PARSE_TOKEN_ID_ENTROPY_RESEED:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_ENTROPY_RESEED);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->entropy_reseed)) {
                trial_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_RET_BITS:
            if (!trial_repr.use_count()) {
                OFFL_FUNC_ERR("out of place {}", PARSE_TOKEN_STR_RET_BITS);
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(trial_repr->ret_bits_expected)) {
                trial_repr->failed_parse_token = token_id;
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
drbg_testvec_t::push(drbg_testvec_push_params_t& push_params)
{
    drbg_pre_push_params_t      drbg_pre_params;

    this->push_params = push_params;
    if (test_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as test representative vector is empty");
        return true;
    }

    drbg = dflt_drbg_get(pre_params.instance());
    hw_started = true;

    FOR_EACH_TEST_REPR(test_repr) {
        FOR_EACH_TRIAL_REPR(test_repr, trial_repr) {
            trial_repr->push_failure = false;

            /*
             * If there were parse errors, skip this representative.
             */
            if ((test_repr->failed_parse_token != PARSE_TOKEN_ID_VOID) ||
                (trial_repr->failed_parse_token != PARSE_TOKEN_ID_VOID)) {

                OFFL_FUNC_ERR("Parser error on test token {} or trial token {}",
                              test_repr->failed_parse_token,
                              trial_repr->failed_parse_token);
                trial_repr->push_failure = true;
                continue;
            }

            if (!drbg->pre_push(drbg_pre_params)) {
                OFFL_FUNC_ERR("failed crypto_drbg pre_push");
                trial_repr->push_failure = true;

            } else {
                if (!trial_execute(trial_repr.get(), drbg)) {
                    OFFL_FUNC_ERR("failed trial_execute");
                    trial_repr->push_failure = true;
                }
            }

        } END_FOR_EACH_TRIAL_REPR(test_repr, trial_repr)

    } END_FOR_EACH_TEST_REPR(test_repr)

    return true;
}

/*
 * Execute a trial
 */
bool
drbg_testvec_t::trial_execute(drbg_trial_repr_t *trial_repr,
                              drbg_t *drbg)
{
    drbg_push_inst_params_t     inst_params;
    drbg_push_uninst_params_t   uninst_params;
    drbg_push_gen_params_t      gen_params;
    bool                        predict_resist_flag;
    bool                        success;

    /*
     * Per DRGBVS, 
     * case A: PredictionResistance=true, each trial runs the following sequence:
     *   1) Instantiate DRBG
     *   2) Generate (do not keep result)
     *   3) Generate (keep result)
     *   4) Uninstantiate
     *
     * case B: PredictionResistance=false
     *   1) Instantiate DRBG
     *   2) Reseed
     *   3) Generate (do not keep result)
     *   4) Generate (keep result)
     *   5) Uninstantiate
     */
    predict_resist_flag = trial_repr->test_repr->predict_resist_flag;
    success = drbg->push(inst_params.psnl_str(trial_repr->psnl_str).
                                     entropy(trial_repr->entropy).
                                     nonce(trial_repr->nonce).
                                     predict_resist_flag(predict_resist_flag));
    if (success) {
        trial_repr->add_input_vec->line_set(0);
        trial_repr->entropy_pr_vec->line_set(0);
        success = drbg->push(gen_params.add_input(trial_repr->add_input_vec).
                                        add_input_reseed(trial_repr->add_input_reseed).
                                        entropy_pr(trial_repr->entropy_pr_vec).
                                        entropy_reseed(trial_repr->entropy_reseed).
                                        output(trial_repr->ret_bits_actual).
                                        predict_resist_req(predict_resist_flag));
    }
    if (success) {
        trial_repr->add_input_vec->line_advance();
        trial_repr->entropy_pr_vec->line_advance();
        success = drbg->push(gen_params.add_input_reseed(nullptr).
                                        entropy_reseed(nullptr));
    }   

    drbg->push(uninst_params);
    return success;
}

/*
 *
 */
bool 
drbg_testvec_t::post_push(void)
{
    drbg_push_reinst_params_t   reinst_params;

    /*
     * Other features may depend on a working DRBG so
     * reinstantiate it with default parameters.
     */
    if (drbg) {
        drbg->push(reinst_params);
    }
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool 
drbg_testvec_t::completion_check(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_TEST_REPR(test_repr) {
            FOR_EACH_TRIAL_REPR(test_repr, trial_repr) {

                /*
                 * All DRBG operations were synchronously completed so
                 * no further calls to DRBG are required.
                 */
                if (trial_repr->push_failure) {
                    num_test_failures++;
                    continue;
                }

            } END_FOR_EACH_TRIAL_REPR(test_repr, trial_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("drbg_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Test result full verification
 */
bool 
drbg_testvec_t::full_verify(void)
{
    return completion_check();
}


/*
 * Generate testvector response output file
 */
void 
drbg_testvec_t::rsp_file_output(void)
{
    if (hw_started && rsp_output) {
        FOR_EACH_TEST_REPR(test_repr) {

            rsp_output->str(PARSE_STR_SHA_PREFIX, test_repr->sha_algo,
                            PARSE_STR_SHA_SUFFIX);
            rsp_output->str(PARSE_STR_PREDICT_PREFIX,
                            test_repr->predict_resist_flag ? "True" : "False",
                            PARSE_STR_PREDICT_SUFFIX);
            rsp_output->dec(PARSE_STR_ENTROPY_LEN_PREFIX, test_repr->entropy_nbits,
                            PARSE_STR_ENTROPY_LEN_SUFFIX);
            rsp_output->dec(PARSE_STR_NONCE_LEN_PREFIX, test_repr->nonce_nbits,
                            PARSE_STR_NONCE_LEN_SUFFIX);
            rsp_output->dec(PARSE_STR_PSNL_STR_LEN_PREFIX, test_repr->psnl_str_nbits,
                            PARSE_STR_PSNL_STR_LEN_SUFFIX);
            rsp_output->dec(PARSE_STR_ADD_INPUT_LEN_PREFIX, test_repr->add_input_nbits,
                            PARSE_STR_ADD_INPUT_LEN_SUFFIX);
            rsp_output->dec(PARSE_STR_RET_BITS_LEN_PREFIX, test_repr->ret_nbits,
                            PARSE_STR_RET_BITS_LEN_SUFFIX);

            FOR_EACH_TRIAL_REPR(test_repr, trial_repr) {

                rsp_output->dec(PARSE_STR_COUNT_PREFIX, trial_repr->count);
                rsp_output->hex_bn(PARSE_STR_ENTROPY_PREFIX, trial_repr->entropy);
                rsp_output->hex_bn(PARSE_STR_NONCE_PREFIX, trial_repr->nonce);
                rsp_output->hex_bn(PARSE_STR_PSNL_STR_PREFIX, trial_repr->psnl_str);
                trial_repr->add_input_vec->line_set(0);
                rsp_output->hex_bn(PARSE_STR_ADD_INPUT_PREFIX, trial_repr->add_input_vec);
                trial_repr->entropy_pr_vec->line_set(0);
                rsp_output->hex_bn(PARSE_STR_ENTROPY_PR_PREFIX, trial_repr->entropy_pr_vec);
                trial_repr->add_input_vec->line_advance();
                rsp_output->hex_bn(PARSE_STR_ADD_INPUT_PREFIX, trial_repr->add_input_vec);
                trial_repr->entropy_pr_vec->line_advance();
                rsp_output->hex_bn(PARSE_STR_ENTROPY_PR_PREFIX, trial_repr->entropy_pr_vec);
                rsp_output->hex_bn(PARSE_STR_RET_BITS_PREFIX, trial_repr->ret_bits_actual,
                                   PARSE_STR_RET_BITS_SUFFIX);

            } END_FOR_EACH_TRIAL_REPR(test_repr, trial_repr)
        } END_FOR_EACH_TEST_REPR(test_repr)
    }

    if (rsp_output) {
        delete rsp_output;
        rsp_output = nullptr;
    }
}

} // namespace crypto_drbg
