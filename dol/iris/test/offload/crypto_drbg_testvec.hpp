#ifndef _CRYPTO_DRBG_TESTVEC_HPP_
#define _CRYPTO_DRBG_TESTVEC_HPP_

#include <stdint.h>
#include <string>
#include <memory>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <openssl/ssl.h>
#include "logger.hpp"
#include "offload_base_params.hpp"
#include "testvec_parser.hpp"
#include "crypto_drbg.hpp"

using namespace tests;

namespace crypto_drbg {

class drbg_test_repr_t;
class drbg_trial_repr_t;
class drbg_testvec_t;

/*
 * Emulate named parameters support for drbg_testvec_t constructor
 */
class drbg_testvec_params_t
{
public:

    drbg_testvec_params_t() {}

    drbg_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    offload_base_params_t& base_params(void) { return base_params_; }

private:
    offload_base_params_t       base_params_;

};

/*
 * Emulate named parameters support for drbg_testvec_t pre_push
 */
class drbg_testvec_pre_push_params_t
{
public:

    drbg_testvec_pre_push_params_t() :
        instance_(DRBG_INSTANCE0)
    {
    }

    drbg_testvec_pre_push_params_t&
    scripts_dir(const string& scripts_dir)
    {
        scripts_dir_.assign(scripts_dir);
        return *this;
    }
    drbg_testvec_pre_push_params_t&
    testvec_fname(const string& testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }
    drbg_testvec_pre_push_params_t&
    instance(drbg_instance_t instance)
    {
        instance_ = instance;
        return *this;
    }

    string& scripts_dir(void) { return scripts_dir_; }
    string& testvec_fname(void) { return testvec_fname_; }
    drbg_instance_t instance(void) { return instance_; }

private:
    string                      scripts_dir_;
    string                      testvec_fname_;
    drbg_instance_t             instance_;
};

/*
 * Emulate named parameters support for drbg_testvec_t push
 */
class drbg_testvec_push_params_t
{
public:

    drbg_testvec_push_params_t() :
        random_num_select_(0)
    {
    }

    drbg_testvec_push_params_t&
    random_num_select(uint32_t random_num_select)
    {
        random_num_select_ = random_num_select;
        return *this;
    }

    uint32_t random_num_select(void) { return random_num_select_; }

private:
    uint32_t                    random_num_select_;
};

/*
 * Crypto DRBG test vector
 */
class drbg_testvec_t {

public:
    drbg_testvec_t(drbg_testvec_params_t& params);
    ~drbg_testvec_t();

    friend class drbg_test_repr_t;
    friend class drbg_trial_repr_t;

    bool pre_push(drbg_testvec_pre_push_params_t& pre_params);
    bool push(drbg_testvec_push_params_t& push_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    void rsp_file_output(void);

private:
    bool shall_execute(drbg_test_repr_t *test_repr);
    bool trial_execute(drbg_trial_repr_t *trial_repr,
                       drbg_t *drbg);

    drbg_testvec_params_t        testvec_params;
    drbg_testvec_pre_push_params_t pre_params;
    drbg_testvec_push_params_t   push_params;
    testvec_parser_t            *testvec_parser;
    testvec_output_t            *rsp_output;
    vector<shared_ptr<drbg_test_repr_t>> test_repr_vec;

    drbg_t                      *drbg;
    uint32_t                    num_test_failures;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Crypto DRBG test representative
 */
class drbg_test_repr_t {

public:
    drbg_test_repr_t(drbg_testvec_t& drbg_testvec) :
        drbg_testvec(drbg_testvec),
        entropy_nbits(0),
        nonce_nbits(0),
        psnl_str_nbits(0),
        add_input_nbits(0),
        ret_nbits(0),
        predict_resist_flag(false),
        failed_parse_token(PARSE_TOKEN_ID_VOID)
    {
    }

    ~drbg_test_repr_t()
    {
        if (drbg_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (drbg_testvec.test_success || !drbg_testvec.hw_started) {
            }
        }
    }

    friend class drbg_testvec_t;
    friend class drbg_trial_repr_t;

private:
    drbg_testvec_t&             drbg_testvec;
    string                      sha_algo;
    u_long                      entropy_nbits;
    u_long                      nonce_nbits;
    u_long                      psnl_str_nbits;
    u_long                      add_input_nbits;
    u_long                      ret_nbits;
    bool                        predict_resist_flag;
    parser_token_id_t           failed_parse_token;
    vector<shared_ptr<drbg_trial_repr_t>> trial_repr_vec;
};

/*
 * Number of instances for certain trial representative fields
 */
enum {
    DRBG_TRIAL_NUM_ADD_INPUT_FIELDS     = 2,
    DRBG_TRIAL_NUM_ENTROPY_PR_FIELDS    = 2,
};

/*
 * Crypto DRBG trial representative
 */
class drbg_trial_repr_t {

public:
    drbg_trial_repr_t(drbg_testvec_t& drbg_testvec,
                      drbg_test_repr_t *test_repr) :
        drbg_testvec(drbg_testvec),
        test_repr(test_repr),
        count(0),
        failed_parse_token(PARSE_TOKEN_ID_VOID),
        push_failure(false),
        verify_failure(false)
    {
        uint32_t        entropy_nbytes;
        uint32_t        nbytes;

        entropy_nbytes = CRYPTO_CTL_DRBG_CEIL_b2B(test_repr->entropy_nbits);
        assert(entropy_nbytes);

        entropy = new dp_mem_t(1, entropy_nbytes,
                               DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                               0, DP_MEM_ALLOC_NO_FILL);
        entropy->content_size_set(0);

        nbytes = CRYPTO_CTL_DRBG_CEIL_b2B(test_repr->nonce_nbits);
        nonce = new dp_mem_t(1, nbytes ? nbytes : entropy_nbytes,
                             DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                             0, DP_MEM_ALLOC_NO_FILL);
        nonce->content_size_set(0);

        nbytes = CRYPTO_CTL_DRBG_CEIL_b2B(test_repr->psnl_str_nbits);
        psnl_str = new dp_mem_t(1, nbytes ? nbytes : entropy_nbytes,
                                DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                0, DP_MEM_ALLOC_NO_FILL);
        psnl_str->content_size_set(0);

        nbytes = CRYPTO_CTL_DRBG_CEIL_b2B(test_repr->add_input_nbits);
        add_input_vec = new dp_mem_t(DRBG_TRIAL_NUM_ADD_INPUT_FIELDS,
                                     nbytes ? nbytes : entropy_nbytes,
                                     DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                     0, DP_MEM_ALLOC_NO_FILL);
        add_input_vec->content_size_set(0);
        add_input_reseed = new dp_mem_t(1, nbytes ? nbytes : entropy_nbytes,
                                        DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                        0, DP_MEM_ALLOC_NO_FILL);
        add_input_reseed->content_size_set(0);

        entropy_pr_vec = new dp_mem_t(DRBG_TRIAL_NUM_ENTROPY_PR_FIELDS,
                                      entropy_nbytes,
                                      DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                      0, DP_MEM_ALLOC_NO_FILL);
        entropy_pr_vec->content_size_set(0);
        entropy_reseed = new dp_mem_t(1, entropy_nbytes,
                                      DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                      0, DP_MEM_ALLOC_NO_FILL);
        entropy_reseed->content_size_set(0);

        nbytes = CRYPTO_CTL_DRBG_CEIL_b2B(test_repr->ret_nbits);
        ret_bits_expected = new dp_mem_t(1, nbytes ? nbytes : entropy_nbytes,
                                DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                                0, DP_MEM_ALLOC_NO_FILL);
        ret_bits_expected->content_size_set(0);
        ret_bits_actual = new dp_mem_t(1, nbytes ? nbytes : entropy_nbytes,
                              DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HBM,
                              0, DP_MEM_ALLOC_FILL_ZERO);
        ret_bits_actual->content_size_set(0);
    }

    ~drbg_trial_repr_t()
    {
        if (drbg_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (drbg_testvec.test_success || !drbg_testvec.hw_started) {
                if (entropy_reseed) delete entropy_reseed;
                if (entropy_pr_vec) delete entropy_pr_vec;
                if (add_input_reseed) delete add_input_reseed;
                if (add_input_vec) delete add_input_vec;
                if (psnl_str) delete psnl_str;
                if (nonce) delete nonce;
                if (entropy) delete entropy;
                if (ret_bits_expected) delete ret_bits_expected;
                if (ret_bits_actual) delete ret_bits_actual;
            }
        }
    }

    friend class drbg_testvec_t;

private:
    drbg_testvec_t&             drbg_testvec;
    drbg_test_repr_t            *test_repr;
    u_long                      count;
    dp_mem_t                    *entropy;
    dp_mem_t                    *nonce;
    dp_mem_t                    *psnl_str;
    dp_mem_t                    *add_input_vec;
    dp_mem_t                    *add_input_reseed;
    dp_mem_t                    *entropy_pr_vec;        // for prediction resistance
    dp_mem_t                    *entropy_reseed;
    dp_mem_t                    *ret_bits_expected;
    dp_mem_t                    *ret_bits_actual;
    parser_token_id_t           failed_parse_token;
    uint32_t                    push_failure    : 1,
                                verify_failure  : 1;
};

} // namespace crypto_drbg

#endif   // _CRYPTO_DRBG_TESTVEC_HPP_

