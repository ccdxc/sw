#include "crypto_ecdsa_testvec.hpp"
#include "testvec_output.hpp"
#include "crypto_ecdsa.hpp"

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
#define PARSE_STR_P_PREFIX              "[P-"
#define PARSE_STR_P_SUFFIX              ","
#define PARSE_STR_B_PREFIX              "[B-"
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
#define PARSE_STR_K_PREFIX              "[K-"
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
    testvec_parse_params_t      params;
    string                      result;
    parser_token_id_t           token_id;
    token_parser_t              token_parser;
    u_long                      P_bits_len = 0;
    u_long                      P_bytes_len = 0;

    OFFL_FUNC_INFO("test vector file {}", pre_params.testvec_fname());
    this->pre_params = pre_params;
    hw_started = false;
    test_success = false;

    /*
     * For ease of parsing, consider brackets as whitespaces;
     * and "=" and "-" as token delimiters
     */
    token_parser.extra_whitespaces_add("[]");
    token_parser.extra_delims_add("=-");
    testvec_parser = new testvec_parser_t(pre_params.scripts_dir(),
                                          pre_params.testvec_fname(),
                                          token_parser, token2id_map);
    while (!testvec_parser->eof()) {

        token_id = testvec_parser->parse(params.skip_unknown_token(true));
        switch (token_id) {

        case PARSE_TOKEN_ID_EOF:
        case PARSE_TOKEN_ID_P:
        case PARSE_TOKEN_ID_B:
        case PARSE_TOKEN_ID_K:

            /*
             * EOF or a new P section terminates the current curve representative
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
            curve_repr = make_shared<ecdsa_curve_repr_t>(*this, P_bytes_len);
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
             * Create Openssl friendly sha_algo string
             */
            curve_repr->sha_algo.assign(PARSE_TOKEN_STR_SHA + curve_repr->sha);
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
            msg_repr = make_shared<ecdsa_msg_repr_t>(*this, P_bytes_len);
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

        case PARSE_TOKEN_ID_S:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place S");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->s)) {
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

        case PARSE_TOKEN_ID_R:
            if (!curve_repr.use_count() || !msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place R");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->r)) {
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
    this->push_params = push_params;
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
    return true;
}


/*
 * Test result full verification
 */
bool 
ecdsa_testvec_t::full_verify(void)
{
    return true;
}


/*
 * Generate testvector response output file
 */
void 
ecdsa_testvec_t::rsp_file_output(const string& mem_type_str)
{
    testvec_output_t    *rsp_output;
    uint32_t            P_bits_len;
    bool                failure;

    rsp_output = new testvec_output_t(pre_params.scripts_dir(),
                                      pre_params.testvec_fname(),
                                      mem_type_str);
    if (true /*hw_started*/) {
        FOR_EACH_CURVE_REPR(curve_repr) {

            P_bits_len = curve_repr->P_bytes_len * BITS_PER_BYTE;
            rsp_output->dec(PARSE_STR_P_PREFIX, P_bits_len,
                            PARSE_STR_P_SUFFIX, false);
            rsp_output->str(PARSE_STR_SHA_PREFIX, curve_repr->sha,
                            PARSE_STR_SHA_SUFFIX);
            FOR_EACH_MSG_REPR(curve_repr, msg_repr) {

                rsp_output->hex_bn(PARSE_STR_MSG_PREFIX, msg_repr->msg);
                if (ecdsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                    rsp_output->hex_bn(PARSE_STR_D_PREFIX, msg_repr->d);
                }
                rsp_output->hex_bn(PARSE_STR_Qx_PREFIX, msg_repr->qx);
                rsp_output->hex_bn(PARSE_STR_Qy_PREFIX, msg_repr->qy);
                if (ecdsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                    rsp_output->hex_bn(PARSE_STR_k_PREFIX, msg_repr->k);
                }
                failure = msg_repr->push_failure        ||
                          msg_repr->compl_failure       ||
                          msg_repr->verify_failure;
                if (ecdsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                    rsp_output->hex_bn(PARSE_STR_R_PREFIX, msg_repr->r);
                    rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->s,
                                       PARSE_STR_S_SUFFIX);
                } else {
                    rsp_output->hex_bn(PARSE_STR_R_PREFIX, msg_repr->r);
                    rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->s);
                    rsp_output->str(PARSE_STR_RESULT_PREFIX, failure ? "F" : "P",
                                    PARSE_STR_RESULT_SUFFIX);
                }

            } END_FOR_EACH_MSG_REPR(curve_repr, msg_repr)
        } END_FOR_EACH_CURVE_REPR(curve_repr)
    }

    delete rsp_output;
}

} // namespace crypto_ecdsa
