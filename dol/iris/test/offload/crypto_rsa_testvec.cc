#include "crypto_rsa_testvec.hpp"
#include "crypto_rsa.hpp"

namespace crypto_rsa {

#define FOR_EACH_KEY_REPR(key_repr)                                     \
    for (uint32_t k = 0; k < key_repr_vec.size(); k++) {                \
        shared_ptr<rsa_key_repr_t> key_repr(key_repr_vec.at(k));

#define END_FOR_EACH_KEY_REPR(key_repr)                                 \
        key_repr.reset();                                               \
    }

#define FOR_EACH_MSG_REPR(key_repr, msg_repr)                           \
    for (uint32_t m = 0; m < key_repr->msg_repr_vec.size(); m++) {      \
        shared_ptr<rsa_msg_repr_t> msg_repr(key_repr->msg_repr_vec.at(m));
        
#define END_FOR_EACH_MSG_REPR(key_repr, msg_repr)                       \
        msg_repr.reset();                                               \
    }


/*
 * Constructor
 */
rsa_testvec_t::rsa_testvec_t(rsa_testvec_params_t& params) :

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
rsa_testvec_t::~rsa_testvec_t()
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
rsa_testvec_t::pre_push(rsa_testvec_pre_push_params_t& pre_params)
{
    vector<testvec_parse_token_t>   token_vec;
    parser_token_mask_t             mod_mask;
    parser_token_mask_t             end_mask;
    parser_token_mask_t             msg_mask;
    parser_token_mask_t             done_mask;
    u_long                          modulus_bits_len;
    u_long                          modulus_bytes_len;
    bool                            success = false;

    OFFL_FUNC_INFO("test vector file {}", pre_params.testvec_fname());
    hw_started = false;
    test_success = false;

    testvec_parser = new testvec_parser_t(pre_params.testvec_fname());
    while (!testvec_parser->eof()) {

        /*
         * First, find modulus length.
         * Note: testvec_parser would strip all white spaces (including
         * brackets) from a line prior to parsing, so tokens should
         * exclude such characters.
         */
        token_vec.clear();
        token_vec.push_back(testvec_parse_token_t("mod=",
                                    &testvec_parser_t::line_parse_ulong,
                                    &modulus_bits_len));
        mod_mask.assign(testvec_parser->parse(token_vec, true));
        if (mod_mask.empty() || (modulus_bits_len == 0)) {
            OFFL_FUNC_ERR("failed to find modulus length");
            goto done;
        }

        /*
         * Parse for e/n/d which are expected to also be at the front
         * of the test vector file
         */
        modulus_bytes_len = (modulus_bits_len + BITS_PER_BYTE - 1) /
                            BITS_PER_BYTE;
        shared_ptr<rsa_key_repr_t> key_repr = 
                   make_shared<rsa_key_repr_t>(*this, modulus_bytes_len);

        token_vec.clear();
        token_vec.push_back(testvec_parse_token_t("n=",
                                    &testvec_parser_t::line_parse_hex_bn,
                                    key_repr->n));
        token_vec.push_back(testvec_parse_token_t("e=",
                                    &testvec_parser_t::line_parse_hex_bn,
                                    key_repr->e));
        token_vec.push_back(testvec_parse_token_t("d=",
                                    &testvec_parser_t::line_parse_hex_bn,
                                    key_repr->d));
        done_mask.fill(3);
        end_mask.assign(testvec_parser->parse(token_vec));
        if (!end_mask.eq(done_mask)) {
            key_repr.reset();
            OFFL_FUNC_ERR("failed to find one of e/n/d");
            goto done;
        }

        /*
         * Now parse the messages
         */
        done_mask.fill(3);
        while (true) {
            shared_ptr<rsa_msg_repr_t> msg_repr = 
                       make_shared<rsa_msg_repr_t>(*this, modulus_bytes_len);
            token_vec.clear();
            token_vec.push_back(testvec_parse_token_t("SHAAlg=",
                                        &testvec_parser_t::line_parse_string,
                                        &msg_repr->sha_algo));
            token_vec.push_back(testvec_parse_token_t("Msg=",
                                        &testvec_parser_t::line_parse_hex_bn,
                                        msg_repr->msg));
            token_vec.push_back(testvec_parse_token_t("S=",
                                        &testvec_parser_t::line_parse_hex_bn,
                                        msg_repr->sig_expected));
            msg_mask.assign(testvec_parser->parse(token_vec));
            if (!msg_mask.eq(done_mask)) {
                msg_repr.reset();
                if (msg_mask.empty()) {
                     break;
                }
                OFFL_FUNC_ERR("failed to find one of SHAAlg/Msg/S");
                goto done;
            }

            key_repr->msg_repr_vec.push_back(move(msg_repr));
        }

        OFFL_FUNC_INFO("modulus_bytes_len {} with {} msg representative vectors",
                       modulus_bytes_len, key_repr->msg_repr_vec.size());
        key_repr_vec.push_back(move(key_repr));
    }

    OFFL_FUNC_INFO("with {} key representative vectors", key_repr_vec.size());
    success = true;

done:
    return success;
}

/*
 * Initiate the test
 */
bool 
rsa_testvec_t::push(rsa_testvec_push_params_t& push_params)
{
    rsa_params_t                rsa_params;
    rsa_sig_pre_push_params_t   sig_pre_params;
    rsa_sig_push_params_t       sig_push_params;

    this->push_params = push_params;
    if (key_repr_vec.empty()) {
        OFFL_FUNC_INFO("nothing to do as key representative vector is empty");
        return true;
    }

    hw_started = true;
    rsa_params.base_params(testvec_params.base_params()).
               acc_ring(push_params.rsa_ring()).
               push_type(push_params.push_type()).
               seq_qid(push_params.seq_qid());

    FOR_EACH_KEY_REPR(key_repr) {
        sig_pre_params.key_create_type(RSA_KEY_CREATE_PRIV_SIG_GEN).
                        n(key_repr->n).
                        d(key_repr->d).
                        e(key_repr->e);
        rsa_params.key_idx(CRYPTO_ASYM_KEY_IDX_INVALID).
                   key_idx_shared(false);
        FOR_EACH_MSG_REPR(key_repr, msg_repr) {

            if (!msg_repr->crypto_rsa) {
                msg_repr->crypto_rsa = new rsa_t(rsa_params);
            }
            sig_pre_params.msg(msg_repr->msg).
                           hash_algo(msg_repr->sha_algo);
            if (!msg_repr->crypto_rsa->pre_push(sig_pre_params)) {
                OFFL_FUNC_ERR("failed crypto_rsa pre_push");
                return false;
            }

            sig_push_params.sig_expected(msg_repr->sig_expected).
                            sig_actual(msg_repr->sig_actual);
            if (!msg_repr->crypto_rsa->push(sig_push_params)) {
                OFFL_FUNC_ERR("failed crypto_rsa push");
                return false;
            }

            /*
             * All msg_repr's of the same key_repr share the same key
             */
            rsa_params.key_idx(msg_repr->crypto_rsa->key_idx_get()).
                       key_idx_shared(true);

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
 * Test result verification (fast and non-blocking)
 *
 */
bool 
rsa_testvec_t::fast_verify(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_KEY_REPR(key_repr) {
            FOR_EACH_MSG_REPR(key_repr, msg_repr) {

              if (!msg_repr->crypto_rsa->fast_verify()) {
                  num_test_failures++;
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
 * Test result verification (full and possibly blocking)
 */
bool 
rsa_testvec_t::full_verify(void)
{
    num_test_failures = 0;
    if (hw_started) {
        FOR_EACH_KEY_REPR(key_repr) {
            FOR_EACH_MSG_REPR(key_repr, msg_repr) {

                if (!msg_repr->crypto_rsa->full_verify()) {
                    num_test_failures++;
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

} // namespace crypto_rsa
