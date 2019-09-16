#include "crypto_ecdsa_testvec.hpp"
#include "testvec_output.hpp"
#include "crypto_ecdsa.hpp"
#include "utils.hpp"

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) prior to parsing.
 */
#define PARSE_TOKEN_STR_RESULT          "Result"
#define PARSE_TOKEN_STR_P               "P"
#define PARSE_TOKEN_STR_B               "B"
#define PARSE_TOKEN_STR_D               "d"
#define PARSE_TOKEN_STR_SHA             "SHA"
#define PARSE_TOKEN_STR_MSG             "Msg"
#define PARSE_TOKEN_STR_S               "S"
#define PARSE_TOKEN_STR_Qx              "Qx"
#define PARSE_TOKEN_STR_Qy              "Qy"
#define PARSE_TOKEN_STR_k               "k"
#define PARSE_TOKEN_STR_K               "K"
#define PARSE_TOKEN_STR_R               "R"

/*
 * Unstripped versions
 */
#define PARSE_STR_RESULT_PREFIX         "Result = "
#define PARSE_STR_RESULT_SUFFIX         "\n"
#define PARSE_STR_CURVE_PREFIX          "["
#define PARSE_STR_CURVE_SUFFIX           ","
#define PARSE_STR_P_PREFIX              "P-"
#define PARSE_STR_B_PREFIX              "B-"
#define PARSE_STR_B_SUFFIX              ","
#define PARSE_STR_D_PREFIX              "d = "
#define PARSE_STR_SHA_PREFIX            "SHA-"
#define PARSE_STR_SHA_SUFFIX            "]\n"
#define PARSE_STR_MSG_PREFIX            "Msg = "
#define PARSE_STR_S_PREFIX              "S = "
#define PARSE_STR_S_SUFFIX              "\n"
#define PARSE_STR_Qx_PREFIX             "Qx = "
#define PARSE_STR_Qy_PREFIX             "Qy = "
#define PARSE_STR_k_PREFIX              "k = "
#define PARSE_STR_K_PREFIX              "K-"
#define PARSE_STR_K_SUFFIX              ","
#define PARSE_STR_R_PREFIX              "R = "

namespace crypto_ecdsa {

#define FOR_EACH_CURVE_REPR(curve_repr)                                   \
    for (uint32_t k = 0; k < curve_repr_vec.size(); k++) {                \
        auto curve_repr = curve_repr_vec.at(k);

#define END_FOR_EACH_CURVE_REPR(curve_repr)                               \
    }

#define FOR_EACH_MSG_REPR(curve_repr, msg_repr)                           \
    for (uint32_t m = 0; m < curve_repr->msg_repr_vec.size(); m++) {      \
        auto msg_repr = curve_repr->msg_repr_vec.at(m);
        
#define END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)                       \
    }


enum {
    PARSE_TOKEN_ID_RESULT       = PARSE_TOKEN_ID_USER,
    PARSE_TOKEN_ID_P,
    PARSE_TOKEN_ID_B,
    PARSE_TOKEN_ID_K,
    PARSE_TOKEN_ID_D,
    PARSE_TOKEN_ID_SHA,
    PARSE_TOKEN_ID_MSG,
    PARSE_TOKEN_ID_S,
    PARSE_TOKEN_ID_Qx,
    PARSE_TOKEN_ID_Qy,
    PARSE_TOKEN_ID_k,
    PARSE_TOKEN_ID_R,
};

const static map<string,parser_token_id_t>      token2id_map =
{
    {PARSE_TOKEN_STR_RESULT,    PARSE_TOKEN_ID_RESULT},
    {PARSE_TOKEN_STR_P,         PARSE_TOKEN_ID_P},
    {PARSE_TOKEN_STR_B,         PARSE_TOKEN_ID_B},
    {PARSE_TOKEN_STR_K,         PARSE_TOKEN_ID_K},
    {PARSE_TOKEN_STR_D,         PARSE_TOKEN_ID_D},
    {PARSE_TOKEN_STR_SHA,       PARSE_TOKEN_ID_SHA},
    {PARSE_TOKEN_STR_MSG,       PARSE_TOKEN_ID_MSG},
    {PARSE_TOKEN_STR_S,         PARSE_TOKEN_ID_S},
    {PARSE_TOKEN_STR_Qx,        PARSE_TOKEN_ID_Qx},
    {PARSE_TOKEN_STR_Qy,        PARSE_TOKEN_ID_Qy},
    {PARSE_TOKEN_STR_k,         PARSE_TOKEN_ID_k},
    {PARSE_TOKEN_STR_R,         PARSE_TOKEN_ID_R},
};

/*
 * Constructor
 */
ecdsa_testvec_t::ecdsa_testvec_t(ecdsa_testvec_params_t& params) :

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
ecdsa_testvec_t::~ecdsa_testvec_t()
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
ecdsa_testvec_t::pre_push(ecdsa_testvec_pre_push_params_t& pre_params)
{
    shared_ptr<ecdsa_curve_repr_t> curve_repr;
    shared_ptr<ecdsa_msg_repr_t>  msg_repr;
    const char                  *curve_prefix;
    testvec_parse_params_t      params;
    string                      result;
    parser_token_id_t           token_id;
    token_parser_t              token_parser;
    u_long                      P_bits_len = 0;
    u_long                      P_bytes_len = 0;
    u_long                      P_expanded_len = 0;

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
     * and "=" and "-" as token delimiters
     */
    token_parser.extra_whitespaces_add("[]");
    token_parser.extra_delims_add("=-");
    testvec_parser = new testvec_parser_t(pre_params.scripts_dir(),
                                          pre_params.testvec_fname(),
                                          token_parser, token2id_map, rsp_output);
    curve_prefix = PARSE_STR_P_PREFIX;
    while (!testvec_parser->eof()) {

        token_id = testvec_parser->parse(params.skip_unknown_token(true).
                                                output_header_comments(true));
        switch (token_id) {

        case PARSE_TOKEN_ID_EOF:
        case PARSE_TOKEN_ID_B:
            curve_prefix = PARSE_STR_B_PREFIX;
            goto new_curve;

        case PARSE_TOKEN_ID_K:
            curve_prefix = PARSE_STR_K_PREFIX;
            goto new_curve;

        case PARSE_TOKEN_ID_P:

        new_curve:

            /*
             * EOF or a new P/B/K section terminates the current curve representative
             * and msg representative.
             */
            if (curve_repr.use_count()) {
                if (msg_repr.use_count()) {
                    curve_repr->msg_repr_vec.push_back(move(msg_repr));
                    msg_repr.reset();
                }

                OFFL_FUNC_INFO("P_bytes_len {} with {} msg representative vectors",
                               P_bytes_len, curve_repr->msg_repr_vec.size());
                curve_repr_vec.push_back(move(curve_repr));
                curve_repr.reset();
            }
            if (token_id == PARSE_TOKEN_ID_EOF) {
                break;
            }

            if (!testvec_parser->parse_ulong(&P_bits_len)) {
                OFFL_FUNC_ERR("Failed to parse P");
                goto error;
            }
            if (P_bits_len == 0) {
                OFFL_FUNC_ERR("P cannot be zero");
                goto error;
            }
            P_bytes_len = (P_bits_len + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
            P_expanded_len = ecdsa_expanded_len(P_bytes_len);
            curve_repr = make_shared<ecdsa_curve_repr_t>(*this, P_bytes_len,
                                                         P_expanded_len);
            /*
             * Create Openssl friendly curve name string
             */
            curve_repr->curve_name.assign(curve_prefix + to_string(P_bits_len));
            break;

        case PARSE_TOKEN_ID_SHA:

            if (!curve_repr.use_count()) {
                OFFL_FUNC_ERR("SHA found without a curve representative");
                goto error;
            }
            if (!testvec_parser->parse_string(&curve_repr->sha)) {
                curve_repr->failed_parse_token = token_id;
            }

            /*
             * Create Openssl friendly hash_algo string
             */
            curve_repr->hash_algo.assign(PARSE_TOKEN_STR_SHA + curve_repr->sha);
            break;

        case PARSE_TOKEN_ID_MSG:

            /*
             * Each SHA section begins a new msg representative
             */
            if (!curve_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Msg");
                goto error;
            }
            if (msg_repr.use_count()) {
                curve_repr->msg_repr_vec.push_back(move(msg_repr));
                msg_repr.reset();
            }

            if (P_bytes_len == 0) {
                OFFL_FUNC_ERR("Msg found without P length");
                goto error;
            }
            msg_repr = make_shared<ecdsa_msg_repr_t>(*this, P_bytes_len,
                                                     P_expanded_len);
            if (!testvec_parser->parse_hex_bn(msg_repr->msg)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_D:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place d");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->d)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_R:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place R");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->r_expected)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_S:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place S");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->s_expected)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_Qx:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Qx");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->qx)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_Qy:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Qy");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->qy)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_k:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place k");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->k)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_RESULT:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Result");
                goto error;
            }
            result.clear();
            testvec_parser->parse_string(&result);
            testvec_parser->line_consume_set();
            msg_repr->failure_expected = !result.empty() &&
                                         (result.front() == 'F');
            break;

        default:

            /*
             * Nothing to do as we already instructed parser to skip over
             * unconsumed line.
             */
            break;
        }
    }

    OFFL_FUNC_INFO("with {} curve representative vectors", curve_repr_vec.size());
    return true;

error:
    return false;
}


/*
 * Initiate the test
 */
bool 
ecdsa_testvec_t::push(ecdsa_testvec_push_params_t& push_params)
{
    eng_if::ec_domain_params_t  domain;
    ecdsa_params_t              params;
    ecdsa_pre_push_params_t     pre_params;
    ecdsa_push_params_t         ecdsa_push_params;
    bool                        curve_domain_failure;

    this->push_params = push_params;
    if (curve_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as curve representative vector is empty");
        return true;
    }

    hw_started = true;
    params.base_params(testvec_params.base_params()).
           dma_desc_mem_type(testvec_params.dma_desc_mem_type()).
           msg_digest_mem_type(testvec_params.msg_mem_type()).
           status_mem_type(testvec_params.status_mem_type()).
           acc_ring(push_params.ecdsa_ring()).
           push_type(push_params.push_type()).
           seq_qid(push_params.seq_qid());

    FOR_EACH_CURVE_REPR(curve_repr) {

        /*
         * Obtain curve domain parameters
         */
        curve_domain_failure = 
              !eng_if::ec_domain_params_gen(
                          domain.curve_name(curve_repr->curve_name).
                                 p(curve_repr->p).
                                 n(curve_repr->n).
                                 xg(curve_repr->xg).
                                 yg(curve_repr->yg).
                                 a(curve_repr->a).
                                 b(curve_repr->b));
        /*
         * Domain parameters takes a DMA descriptor, where 
         * p, n, xg, yg, a, b are contiguous fragments of domain_vec.
         */
        curve_repr->domain_vec->content_size_set(
                                crypto_asym::ECDSA_NUM_KEY_DOMAIN_FIELDS *
                                curve_repr->p->content_size_get());
        pre_params.key_create_type(testvec_params.key_create_type()).
                   curve_nid(domain.curve_nid()).
                   domain_vec(curve_repr->domain_vec).
                   hash_algo(curve_repr->hash_algo);

        FOR_EACH_MSG_REPR(curve_repr, msg_repr) {
            msg_repr->push_failure = curve_domain_failure;
            if (curve_domain_failure) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed curve domain parameters for curve {}",
                                       curve_repr->curve_name);
                continue;
            }

            /*
             * If there were parse errors, skip this representative.
             */
            if ((curve_repr->failed_parse_token != PARSE_TOKEN_ID_VOID) ||
                (msg_repr->failed_parse_token != PARSE_TOKEN_ID_VOID)) {

                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected, "Parser error"
                                       " on curve token {} or msg token {}",
                                       curve_repr->failed_parse_token,
                                       msg_repr->failed_parse_token);
                msg_repr->push_failure = true;
                continue;
            }
            if (!msg_repr->crypto_ecdsa) {
                msg_repr->crypto_ecdsa = 
                    new ecdsa_t(params.P_bytes_len(curve_repr->P_bytes_len).
                                       P_expanded_len(curve_repr->P_expanded_len));
            }

            pre_params.d(msg_repr->d).
                       pub_key_vec(msg_repr->pub_key_vec).
                       qx(msg_repr->qx).
                       qy(msg_repr->qy).
                       msg(msg_repr->msg);
            if (!msg_repr->crypto_ecdsa->pre_push(pre_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_ecdsa pre_push");
                msg_repr->push_failure = true;

            } else {
                ecdsa_push_params.k_random(msg_repr->k).
                                  sig_expected_vec(msg_repr->sig_expected_vec).
                                  r_expected(msg_repr->r_expected).
                                  s_expected(msg_repr->s_expected).
                                  sig_actual_vec(msg_repr->sig_actual_vec).
                                  r_actual(msg_repr->r_actual).
                                  s_actual(msg_repr->s_actual).
                                  failure_expected(msg_repr->failure_expected);
#ifdef __x86_64__
                /*
                 * failure_expected implies a SigVer test and there would be
                 * no way to suppress verification failure messages from the model
                 * which would mess up jobd sanity runs. Hence, we will skip the
                 * push step altogether to avoid that.
                 */
                if (msg_repr->failure_expected) {
                    msg_repr->push_failure = true;
                    continue;
                }
#endif
                if (!msg_repr->crypto_ecdsa->push(ecdsa_push_params)) {
                    OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                           "failed crypto_ecdsa push");
                    msg_repr->push_failure = true;
                }
            }

        } END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)

    } END_FOR_EACH_CURVE_REPR(curve_repr)

    return true;
}

/*
 *
 */
bool 
ecdsa_testvec_t::post_push(void)
{
    push_params.ecdsa_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool 
ecdsa_testvec_t::completion_check(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_CURVE_REPR(curve_repr) {
            FOR_EACH_MSG_REPR(curve_repr, msg_repr) {

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

                if (!msg_repr->crypto_ecdsa->completion_check()) {
                    msg_repr->compl_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)
        } END_FOR_EACH_CURVE_REPR(curve_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("ecdsa_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Test result full verification
 */
bool 
ecdsa_testvec_t::full_verify(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_CURVE_REPR(curve_repr) {
            FOR_EACH_MSG_REPR(curve_repr, msg_repr) {

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

                if (!msg_repr->crypto_ecdsa->full_verify()) {
                    msg_repr->verify_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)
        } END_FOR_EACH_CURVE_REPR(curve_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("ecdsa_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Generate testvector response output file
 */
void 
ecdsa_testvec_t::rsp_file_output(void)
{
    bool                failure;

    if (hw_started && rsp_output) {
        FOR_EACH_CURVE_REPR(curve_repr) {

            rsp_output->str(PARSE_STR_CURVE_PREFIX, curve_repr->curve_name,
                            PARSE_STR_CURVE_SUFFIX, false);
            rsp_output->str(PARSE_STR_SHA_PREFIX, curve_repr->sha,
                            PARSE_STR_SHA_SUFFIX);
            FOR_EACH_MSG_REPR(curve_repr, msg_repr) {

                rsp_output->hex_bn(PARSE_STR_MSG_PREFIX, msg_repr->msg);
                if (ecdsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                    rsp_output->hex_bn(PARSE_STR_D_PREFIX, msg_repr->d);
                    rsp_output->hex_bn(PARSE_STR_k_PREFIX, msg_repr->k);
                    rsp_output->hex_bn(PARSE_STR_R_PREFIX, msg_repr->r_actual);
                    rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->s_actual,
                                       PARSE_STR_S_SUFFIX);
                } else {
                    rsp_output->hex_bn(PARSE_STR_Qx_PREFIX, msg_repr->qx);
                    rsp_output->hex_bn(PARSE_STR_Qy_PREFIX, msg_repr->qy);

                    failure = msg_repr->push_failure        ||
                              msg_repr->compl_failure       ||
                              msg_repr->verify_failure;
                    rsp_output->hex_bn(PARSE_STR_R_PREFIX, msg_repr->r_expected);
                    rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->s_expected);
                    rsp_output->str(PARSE_STR_RESULT_PREFIX, failure ? "F" : "P",
                                    PARSE_STR_RESULT_SUFFIX);
                }

            } END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)
        } END_FOR_EACH_CURVE_REPR(curve_repr)
    }

    if (rsp_output) {
        delete rsp_output;
        rsp_output = nullptr;
    }
}

} // namespace crypto_ecdsa
