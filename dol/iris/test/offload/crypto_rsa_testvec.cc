#include "crypto_rsa_testvec.hpp"
#include "crypto_rsa.hpp"
#include "utils.hpp"

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) prior to parsing.
 */
#define PARSE_TOKEN_STR_RESULT          "Result"
#define PARSE_TOKEN_STR_MODULUS         "mod"
#define PARSE_TOKEN_STR_N               "n"
#define PARSE_TOKEN_STR_D               "d"
#define PARSE_TOKEN_STR_E               "e"
#define PARSE_TOKEN_STR_SHA_ALGO        "SHAAlg"
#define PARSE_TOKEN_STR_MSG             "Msg"
#define PARSE_TOKEN_STR_S               "S"
#define PARSE_TOKEN_STR_SALT_VAL        "SaltVal"
#define PARSE_TOKEN_STR_SALT_LEN        "SaltLen"

/*
 * Unstripped versions
 */
#define PARSE_STR_RESULT_PREFIX         "Result = "
#define PARSE_STR_RESULT_SUFFIX         "\n"
#define PARSE_STR_MODULUS_PREFIX        "[mod = "
#define PARSE_STR_MODULUS_SUFFIX        "]\n"
#define PARSE_STR_N_PREFIX              "n = "
#define PARSE_STR_N_SUFFIX              "\n"
#define PARSE_STR_D_PREFIX              "d = "
#define PARSE_STR_D_SUFFIX              "\n"
#define PARSE_STR_E_PREFIX              "e = "
#define PARSE_STR_SHA_ALGO_PREFIX       "SHAAlg = "
#define PARSE_STR_MSG_PREFIX            "Msg = "
#define PARSE_STR_S_PREFIX              "S = "
#define PARSE_STR_S_SUFFIX              "\n"
#define PARSE_STR_SALT_VAL_PREFIX       "SaltVal = "
#define PARSE_STR_SALT_VAL_SUFFIX       "\n"
#define PARSE_STR_SALT_LEN_PREFIX       "SaltLen = "

namespace crypto_rsa {

#define FOR_EACH_KEY_REPR(key_repr)                                     \
    for (uint32_t k = 0; k < key_repr_vec.size(); k++) {                \
        auto key_repr = key_repr_vec.at(k);

#define END_FOR_EACH_KEY_REPR(key_repr)                                 \
    }

#define FOR_EACH_MSG_REPR(key_repr, msg_repr)                           \
    for (uint32_t m = 0; m < key_repr->msg_repr_vec.size(); m++) {      \
        auto msg_repr = key_repr->msg_repr_vec.at(m);
        
#define END_FOR_EACH_MSG_REPR(key_repr, msg_repr)                       \
    }


enum {
    PARSE_TOKEN_ID_RESULT       = PARSE_TOKEN_ID_USER,
    PARSE_TOKEN_ID_MODULUS,
    PARSE_TOKEN_ID_N,
    PARSE_TOKEN_ID_D,
    PARSE_TOKEN_ID_E,
    PARSE_TOKEN_ID_SHA_ALGO,
    PARSE_TOKEN_ID_MSG,
    PARSE_TOKEN_ID_S,
    PARSE_TOKEN_ID_SALT_VAL,
    PARSE_TOKEN_ID_SALT_LEN,
};

const static map<string,parser_token_id_t>      token2id_map =
{
    {PARSE_TOKEN_STR_RESULT,    PARSE_TOKEN_ID_RESULT},
    {PARSE_TOKEN_STR_MODULUS,   PARSE_TOKEN_ID_MODULUS},
    {PARSE_TOKEN_STR_N,         PARSE_TOKEN_ID_N},
    {PARSE_TOKEN_STR_D,         PARSE_TOKEN_ID_D},
    {PARSE_TOKEN_STR_E,         PARSE_TOKEN_ID_E},
    {PARSE_TOKEN_STR_SHA_ALGO,  PARSE_TOKEN_ID_SHA_ALGO},
    {PARSE_TOKEN_STR_MSG,       PARSE_TOKEN_ID_MSG},
    {PARSE_TOKEN_STR_S,         PARSE_TOKEN_ID_S},
    {PARSE_TOKEN_STR_SALT_VAL,  PARSE_TOKEN_ID_SALT_VAL},
    {PARSE_TOKEN_STR_SALT_LEN,  PARSE_TOKEN_ID_SALT_LEN},
};

/*
 * Constructor
 */
rsa_testvec_t::rsa_testvec_t(rsa_testvec_params_t& params) :

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
rsa_testvec_t::~rsa_testvec_t()
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
rsa_testvec_t::pre_push(rsa_testvec_pre_push_params_t& pre_params)
{
    shared_ptr<rsa_key_repr_t>  key_repr;
    shared_ptr<rsa_msg_repr_t>  msg_repr;
    testvec_parse_params_t      params;
    string                      result;
    parser_token_id_t           token_id;
    token_parser_t              token_parser;
    u_long                      mod_bits_len = 0;
    u_long                      mod_bytes_len = 0;

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
        case PARSE_TOKEN_ID_N:

            /*
             * EOF or a new n section terminates the current key representative
             * and msg representative.
             */
            if (key_repr.use_count()) {
                if (msg_repr.use_count()) {
                    key_repr->msg_repr_vec.push_back(move(msg_repr));
                    msg_repr.reset();
                }

                OFFL_FUNC_INFO("mod_bytes_len {} with {} msg representative vectors",
                               mod_bytes_len, key_repr->msg_repr_vec.size());
                key_repr_vec.push_back(move(key_repr));
                key_repr.reset();
            }
            if (token_id == PARSE_TOKEN_ID_EOF) {
                break;
            }

            if (mod_bytes_len == 0) {
                OFFL_FUNC_ERR("modulus N found without length");
                goto error;
            }

            key_repr = make_shared<rsa_key_repr_t>(*this, mod_bytes_len);
            if (!testvec_parser->parse_hex_bn(key_repr->n)) {
                key_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_MODULUS:
            if (!testvec_parser->parse_ulong(&mod_bits_len)) {
                key_repr->failed_parse_token = token_id;
            }
            if (mod_bits_len == 0) {
                OFFL_FUNC_ERR("modulus length cannot be zero");
                goto error;
            }
            mod_bytes_len = (mod_bits_len + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
            break;

        case PARSE_TOKEN_ID_E:
            if (rsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                if (!key_repr.use_count()) {
                    OFFL_FUNC_ERR("out of place e for SigGen vector");
                    goto error;
                }
                if (!testvec_parser->parse_hex_bn(key_repr->e)) {
                    key_repr->failed_parse_token = token_id;
                }

            } else {
                if (!msg_repr.use_count()) {
                    OFFL_FUNC_ERR("out of place e for SigVer vector");
                    goto error;
                }
                if (!testvec_parser->parse_hex_bn(msg_repr->e)) {
                    msg_repr->failed_parse_token = token_id;
                }
            }
            break;

        case PARSE_TOKEN_ID_D:
            if (!key_repr.use_count()) {
                OFFL_FUNC_ERR("out of place d");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(key_repr->d)) {
                key_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_SHA_ALGO:

            /*
             * Each SHAAlg section begins a new msg representative
             */
            if (!key_repr.use_count()) {
                OFFL_FUNC_ERR("SHAAlg found without a key representative");
                goto error;
            }
            if (msg_repr.use_count()) {
                key_repr->msg_repr_vec.push_back(move(msg_repr));
                msg_repr.reset();
            }

            msg_repr = make_shared<rsa_msg_repr_t>(*this, mod_bytes_len);
            if (!testvec_parser->parse_string(&msg_repr->sha_algo)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_S:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place S");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->sig_expected)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_MSG:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Msg");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->msg_expected)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_SALT_VAL:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place SaltVal");
                goto error;
            }
            if (!testvec_parser->parse_hex_bn(msg_repr->salt_val)) {
                msg_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_SALT_LEN:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place SaltLen");
                goto error;
            }
            if (!testvec_parser->parse_ulong(&msg_repr->salt_len)) {
                key_repr->failed_parse_token = token_id;
            }
            break;

        case PARSE_TOKEN_ID_RESULT:
            if (!msg_repr.use_count()) {
                OFFL_FUNC_ERR("out of place Result");
                goto error;
            }
            result.clear();
            testvec_parser->parse_string(&result);
            msg_repr->failure_expected = !result.empty() &&
                                         (result.front() == 'F');
            break;

        default:

            /*
             * Nothing to do as we already instructed parser to skip over
             * unknown token
             */
            break;
        }
    }

    OFFL_FUNC_INFO("with {} key representative vectors", key_repr_vec.size());
    return true;

error:
    return false;
}


/*
 * Initiate the test
 */
bool 
rsa_testvec_t::push(rsa_testvec_push_params_t& push_params)
{
    rsa_params_t                rsa_params;
    rsa_pre_push_params_t       rsa_pre_params;
    rsa_push_params_t           rsa_push_params;

    this->push_params = push_params;
    if (key_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as key representative vector is empty");
        return true;
    }

    hw_started = true;
    rsa_params.base_params(testvec_params.base_params()).
               dma_desc_mem_type(testvec_params.dma_desc_mem_type()).
               msg_digest_mem_type(testvec_params.msg_mem_type()).
               status_mem_type(testvec_params.status_mem_type()).
               acc_ring(push_params.rsa_ring()).
               push_type(push_params.push_type()).
               seq_qid(push_params.seq_qid());

    FOR_EACH_KEY_REPR(key_repr) {
        rsa_pre_params.key_create_type(testvec_params.key_create_type()).
                       pad_mode(testvec_params.pad_mode()).
                       d(key_repr->d).
                       e(key_repr->e).
                       n(key_repr->n);

        rsa_params.key_idx(CRYPTO_ASYM_KEY_IDX_INVALID).
                   key_idx_shared(false);
        FOR_EACH_MSG_REPR(key_repr, msg_repr) {
            msg_repr->push_failure = false;

            /*
             * If there were parse errors, skip this representative.
             */
            if ((key_repr->failed_parse_token != PARSE_TOKEN_ID_VOID) ||
                (msg_repr->failed_parse_token != PARSE_TOKEN_ID_VOID)) {

                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected, "Parser error"
                                       " on key token {} or msg token {}",
                                       key_repr->failed_parse_token,
                                       msg_repr->failed_parse_token);
                msg_repr->push_failure = true;
                continue;
            }
            if (!rsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                rsa_pre_params.e(msg_repr->e);
            }

            if (!msg_repr->crypto_rsa) {
                msg_repr->crypto_rsa = new rsa_t(rsa_params);
            }
            rsa_pre_params.msg(msg_repr->msg_expected).
                           hash_algo(msg_repr->sha_algo);
            if (!msg_repr->crypto_rsa->pre_push(rsa_pre_params)) {
                OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                       "failed crypto_rsa pre_push");
                msg_repr->push_failure = true;

            } else {
                rsa_push_params.msg_expected(msg_repr->msg_expected).
                                msg_actual(msg_repr->msg_actual).
                                sig_expected(msg_repr->sig_expected).
                                sig_actual(msg_repr->sig_actual).
                                salt_val(msg_repr->salt_val).
                                salt_len(msg_repr->salt_len).
                                failure_expected(msg_repr->failure_expected);
                if (!msg_repr->crypto_rsa->push(rsa_push_params)) {
                    OFFL_FUNC_ERR_OR_DEBUG(msg_repr->failure_expected,
                                           "failed crypto_rsa push");
                    msg_repr->push_failure = true;
                }
            }

            /*
             * For signing, all msg_repr's of the same key_repr share
             * the same key
             */
            if (rsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                rsa_params.key_idx(msg_repr->crypto_rsa->key_idx_get()).
                           key_idx_shared(true);
            }

        } END_FOR_EACH_MSG_REPR(key_repr, msg_repr)

    } END_FOR_EACH_KEY_REPR(key_repr)

    return true;
}

/*
 *
 */
bool 
rsa_testvec_t::post_push(void)
{
    push_params.rsa_ring()->post_push();
    return true;
}


/*
 * Check status for completion and whether there were errors.
 */
bool 
rsa_testvec_t::completion_check(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_KEY_REPR(key_repr) {
            FOR_EACH_MSG_REPR(key_repr, msg_repr) {

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

                if (!msg_repr->crypto_rsa->completion_check()) {
                    msg_repr->compl_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(key_repr, msg_repr)
        } END_FOR_EACH_KEY_REPR(key_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("rsa_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Test result full verification
 */
bool 
rsa_testvec_t::full_verify(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_KEY_REPR(key_repr) {
            FOR_EACH_MSG_REPR(key_repr, msg_repr) {

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

                if (!msg_repr->crypto_rsa->full_verify()) {
                    msg_repr->verify_failure = true;
                    if (!msg_repr->failure_expected) {
                        num_test_failures++;
                    }
                }

            } END_FOR_EACH_MSG_REPR(key_repr, msg_repr)
        } END_FOR_EACH_KEY_REPR(key_repr)
    }

    if (num_test_failures) {
        OFFL_FUNC_INFO("rsa_testvec_t num_test_failures {}", num_test_failures);
    }
    test_success = hw_started && (num_test_failures == 0);
    return test_success;
}


/*
 * Generate testvector response output file
 */
void 
rsa_testvec_t::rsp_file_output(void)
{
    uint32_t            mod_bits_len;
    uint32_t            new_bits_len;
    bool                failure;

    if (hw_started && rsp_output) {
        mod_bits_len = 0;
        FOR_EACH_KEY_REPR(key_repr) {

            new_bits_len = key_repr->n->content_size_get() * BITS_PER_BYTE;
            if (rsa_key_create_type_is_sign(testvec_params.key_create_type()) ||
                (new_bits_len != mod_bits_len)) {

                rsp_output->dec(PARSE_STR_MODULUS_PREFIX, new_bits_len,
                                PARSE_STR_MODULUS_SUFFIX);
            }
            mod_bits_len = new_bits_len;

            rsp_output->hex_bn(PARSE_STR_N_PREFIX, key_repr->n,
                               PARSE_STR_N_SUFFIX);
            if (rsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                rsp_output->hex_bn(PARSE_STR_E_PREFIX, key_repr->e);
                rsp_output->hex_bn(PARSE_STR_D_PREFIX, key_repr->d,
                                   PARSE_STR_D_SUFFIX);
            }
            FOR_EACH_MSG_REPR(key_repr, msg_repr) {

                failure = msg_repr->push_failure        ||
                          msg_repr->compl_failure       ||
                          msg_repr->verify_failure;
                rsp_output->str(PARSE_STR_SHA_ALGO_PREFIX, msg_repr->sha_algo);
                if (rsa_key_create_type_is_sign(testvec_params.key_create_type())) {
                    rsp_output->hex_bn(PARSE_STR_MSG_PREFIX, msg_repr->msg_expected);
                    if (msg_repr->salt_val->content_size_get()) {
                        rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->sig_actual);
                        rsp_output->hex_bn(PARSE_STR_SALT_VAL_PREFIX, msg_repr->salt_val,
                                           PARSE_STR_SALT_VAL_SUFFIX);
                    } else {
                        rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->sig_actual,
                                           PARSE_STR_S_SUFFIX);
                    }
                } else {
                    rsp_output->hex_bn(PARSE_STR_E_PREFIX, msg_repr->e);
                    rsp_output->hex_bn(PARSE_STR_MSG_PREFIX, msg_repr->msg_expected);
                    rsp_output->hex_bn(PARSE_STR_S_PREFIX, msg_repr->sig_expected);
                    if (msg_repr->salt_val->content_size_get()) {
                        rsp_output->hex_bn(PARSE_STR_SALT_VAL_PREFIX, msg_repr->salt_val);
                    }
                    rsp_output->str(PARSE_STR_RESULT_PREFIX, failure ? "F" : "P",
                                    PARSE_STR_RESULT_SUFFIX);
                }

            } END_FOR_EACH_MSG_REPR(key_repr, msg_repr)
        } END_FOR_EACH_KEY_REPR(key_repr)
    }

    if (rsp_output) {
        delete rsp_output;
        rsp_output = nullptr;
    }
}

} // namespace crypto_rsa
