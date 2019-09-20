#ifndef _CRYPTO_AES_TESTVEC_HPP_
#define _CRYPTO_AES_TESTVEC_HPP_

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
#include "crypto_aes.hpp"

using namespace tests;

namespace crypto_aes {

/*
 * Default CAVP Montecarlo iterations
 */
#define AES_MONTECARLO_ITERS_MAX        100000
#define AES_MONTECARLO_RESULT_EPOCH     1000
#define AES_MONTECARLO_RESULT_LIVENESS  10

#define AES_TESTVEC_MSG_SIZE_MAX        256

class aes_test_repr_t;
class aes_msg_repr_t;
class aes_testvec_t;

/*
 * Emulate named parameters support for aes_testvec_t constructor
 */
class aes_testvec_params_t
{
public:

    aes_testvec_params_t() :
        crypto_symm_type_(crypto_symm::CRYPTO_SYMM_TYPE_AES_CBC),
        montecarlo_type_(crypto_aes::CRYPTO_AES_NOT_MONTECARLO),
        msg_desc_mem_type_(DP_MEM_TYPE_HBM),
        msg_mem_type_(DP_MEM_TYPE_HBM),
        key_mem_type_(DP_MEM_TYPE_HBM),
        iv_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        doorbell_mem_type_(DP_MEM_TYPE_HBM),
        montecarlo_iters_max_(AES_MONTECARLO_ITERS_MAX),
        montecarlo_result_epoch_(AES_MONTECARLO_RESULT_EPOCH)
    {
    }

    aes_testvec_params_t&
    crypto_symm_type(crypto_symm::crypto_symm_type_t crypto_symm_type)
    {
        crypto_symm_type_ = crypto_symm_type;
        return *this;
    }
    aes_testvec_params_t&
    montecarlo_type(crypto_aes::crypto_aes_montecarlo_t montecarlo_type)
    {
        montecarlo_type_ = montecarlo_type;
        return *this;
    }
    aes_testvec_params_t&
    msg_desc_mem_type(dp_mem_type_t msg_desc_mem_type)
    {
        msg_desc_mem_type_ = msg_desc_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    msg_mem_type(dp_mem_type_t msg_mem_type)
    {
        msg_mem_type_ = msg_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    key_mem_type(dp_mem_type_t key_mem_type)
    {
        key_mem_type_ = key_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    iv_mem_type(dp_mem_type_t iv_mem_type)
    {
        iv_mem_type_ = iv_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    doorbell_mem_type(dp_mem_type_t doorbell_mem_type)
    {
        doorbell_mem_type_ = doorbell_mem_type;
        return *this;
    }
    aes_testvec_params_t&
    montecarlo_iters_max(uint32_t montecarlo_iters_max)
    {
        montecarlo_iters_max_ = montecarlo_iters_max;
        return *this;
    }
    aes_testvec_params_t&
    montecarlo_result_epoch(uint32_t montecarlo_result_epoch)
    {
        montecarlo_result_epoch_ = montecarlo_result_epoch;
        return *this;
    }
    aes_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    crypto_symm::crypto_symm_type_t crypto_symm_type(void) { return crypto_symm_type_; }
    crypto_aes::crypto_aes_montecarlo_t montecarlo_type(void) { return montecarlo_type_; }
    dp_mem_type_t msg_desc_mem_type(void) { return msg_desc_mem_type_; }
    dp_mem_type_t msg_mem_type(void) { return msg_mem_type_; }
    dp_mem_type_t key_mem_type(void) { return key_mem_type_; }
    dp_mem_type_t iv_mem_type(void) { return iv_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t doorbell_mem_type(void) { return doorbell_mem_type_; }
    uint32_t montecarlo_iters_max(void) { return montecarlo_iters_max_; }
    uint32_t montecarlo_result_epoch(void) { return montecarlo_result_epoch_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    crypto_symm::crypto_symm_type_t crypto_symm_type_;
    crypto_aes::crypto_aes_montecarlo_t montecarlo_type_;
    dp_mem_type_t               msg_desc_mem_type_;
    dp_mem_type_t               msg_mem_type_;
    dp_mem_type_t               key_mem_type_;
    dp_mem_type_t               iv_mem_type_;
    dp_mem_type_t               status_mem_type_;
    dp_mem_type_t               doorbell_mem_type_;
    uint32_t                    montecarlo_iters_max_;
    uint32_t                    montecarlo_result_epoch_;
    offload_base_params_t       base_params_;

};

/*
 * Emulate named parameters support for aes_testvec_t pre_push
 */
class aes_testvec_pre_push_params_t
{
public:

    aes_testvec_pre_push_params_t() {}

    aes_testvec_pre_push_params_t&
    scripts_dir(const string& scripts_dir)
    {
        scripts_dir_.assign(scripts_dir);
        return *this;
    }
    aes_testvec_pre_push_params_t&
    testvec_fname(const string& testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }
    aes_testvec_pre_push_params_t&
    rsp_fname_suffix(const string& rsp_fname_suffix)
    {
        rsp_fname_suffix_.assign(rsp_fname_suffix);
        return *this;
    }

    string& scripts_dir(void) { return scripts_dir_; }
    string& testvec_fname(void) { return testvec_fname_; }
    string& rsp_fname_suffix(void) { return rsp_fname_suffix_; }

private:
    string                      scripts_dir_;
    string                      testvec_fname_;
    string                      rsp_fname_suffix_;
};

/*
 * Emulate named parameters support for aes_testvec_t push
 */
class aes_testvec_push_params_t
{
public:

    aes_testvec_push_params_t() :
        aes_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    aes_testvec_push_params_t&
    aes_ring(acc_ring_t *aes_ring)
    {
        aes_ring_ = aes_ring;
        return *this;
    }
    aes_testvec_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    aes_testvec_push_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }

    acc_ring_t *aes_ring(void) { return aes_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }

private:
    acc_ring_t                  *aes_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
};

/*
 * Crypto AES Montecarlo result
 */
class aes_montecarlo_result_t {

public:
    aes_montecarlo_result_t() :
        repr_nbytes(0)
    {
    }
    ~aes_montecarlo_result_t()
    {
        clear();
    }

    bool push_back(dp_mem_t *src_result)
    {
        uint32_t    src_nbytes = src_result->content_size_get();
        uint8_t     *result;

        if (src_nbytes &&
            ((repr_nbytes == 0) || (repr_nbytes == src_nbytes))) {

            repr_nbytes = src_nbytes;
            result = new (std::nothrow) uint8_t[src_nbytes];
            if (result) {
                memcpy(result, src_result->read(), src_nbytes);
                result_vec.push_back(result);
                return true;
            }
        }
        return false;
    }

    uint32_t count(void)
    {
         return result_vec.size();
    }

    const uint8_t *result(uint32_t idx)
    {
         return idx < count() ? result_vec.at(idx) : nullptr;
    }

    bool result_to_dp_mem(uint32_t idx,
                          dp_mem_t *dst)
    {
         if (idx < count()) {
             memcpy(dst->read(), result(idx),
                    min(dst->line_size_get(), size()));
             dst->content_size_set(size());
             dst->write_thru();
             return true;
         }
         return false;
    }

    uint32_t size(void)
    {
         return repr_nbytes;
    }

    void clear(void)
    {
        for (size_t idx = 0; idx < count(); idx++) {
            delete [] result_vec.at(idx);
        }
        result_vec.clear();
    }

private:
    vector<uint8_t *>           result_vec;
    uint32_t                    repr_nbytes;
};

/*
 * Crypto AES test vector
 */
class aes_testvec_t {

public:
    aes_testvec_t(aes_testvec_params_t& params);
    ~aes_testvec_t();

    friend class aes_test_repr_t;
    friend class aes_msg_repr_t;

    bool pre_push(aes_testvec_pre_push_params_t& pre_params);
    bool push(aes_testvec_push_params_t& push_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    void rsp_file_output(void);

private:
    bool is_montecarlo(void);
    void montecarlo_cbc_execute(aes_msg_repr_t *msg_repr,
                                crypto_symm::crypto_symm_op_t op);

    aes_testvec_params_t        testvec_params;
    aes_testvec_pre_push_params_t pre_params;
    aes_testvec_push_params_t   push_params;
    testvec_parser_t            *testvec_parser;
    testvec_output_t            *rsp_output;
    vector<shared_ptr<aes_test_repr_t>> test_repr_vec;

    uint32_t                    num_test_failures;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Crypto AES test representative
 */
class aes_test_repr_t {

public:
    aes_test_repr_t(aes_testvec_t& aes_testvec) :
        aes_testvec(aes_testvec),
        op(crypto_symm::CRYPTO_SYMM_OP_ENCRYPT),
        failed_parse_token(PARSE_TOKEN_ID_VOID)
    {
    }

    ~aes_test_repr_t()
    {
        if (aes_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (aes_testvec.test_success || !aes_testvec.hw_started) {
            }
        }
    }

    friend class aes_testvec_t;
    friend class aes_msg_repr_t;

private:
    aes_testvec_t&              aes_testvec;
    crypto_symm::crypto_symm_op_t op;
    parser_token_id_t           failed_parse_token;
    vector<shared_ptr<aes_msg_repr_t>> msg_repr_vec;
};

/*
 * Crypto AES message representative
 */
class aes_msg_repr_t {

public:
    aes_msg_repr_t(aes_testvec_t& aes_testvec) :
        aes_testvec(aes_testvec),
        crypto_aes(nullptr),
        failed_parse_token(PARSE_TOKEN_ID_VOID),
        failure_expected(false),
        push_failure(false),
        compl_failure(false),
        verify_failure(false)
    {
        key = new dp_mem_t(1, CRYPTO_AES_KEY_SIZE_MAX, DP_MEM_ALIGN_SPEC,
                           aes_testvec.testvec_params.key_mem_type(),
                           CRYPTO_SYMM_KEY_ALIGNMENT,
                           DP_MEM_ALLOC_NO_FILL);
        key->content_size_set(0);
        iv = new dp_mem_t(1, CRYPTO_AES_IV_SIZE_MAX, DP_MEM_ALIGN_SPEC,
                          aes_testvec.testvec_params.iv_mem_type(),
                          CRYPTO_SYMM_IV_ALIGNMENT,
                          DP_MEM_ALLOC_NO_FILL);
        iv->content_size_set(0);
        msg_input = new dp_mem_t(1, AES_TESTVEC_MSG_SIZE_MAX,
                                 DP_MEM_ALIGN_SPEC,
                                 aes_testvec.testvec_params.msg_mem_type(),
                                 CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                 DP_MEM_ALLOC_NO_FILL);
        msg_input->content_size_set(0);
        msg_input_tmp = new dp_mem_t(1, AES_TESTVEC_MSG_SIZE_MAX,
                                     DP_MEM_ALIGN_SPEC,
                                     aes_testvec.testvec_params.msg_mem_type(),
                                     CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                     DP_MEM_ALLOC_NO_FILL);
        msg_input_tmp->content_size_set(0);
        msg_output_actual = new dp_mem_t(1, AES_TESTVEC_MSG_SIZE_MAX,
                                         DP_MEM_ALIGN_SPEC,
                                         aes_testvec.testvec_params.msg_mem_type(),
                                         CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                         DP_MEM_ALLOC_FILL_ZERO);
        msg_output_actual->content_size_set(0);

        msg_output_expected = new dp_mem_t(1, AES_TESTVEC_MSG_SIZE_MAX,
                                           DP_MEM_ALIGN_SPEC,
                                           aes_testvec.testvec_params.msg_mem_type(),
                                           CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                           DP_MEM_ALLOC_NO_FILL);
        msg_output_expected->content_size_set(0);

        /*
         * msg_work is a work area for Montecarlo test where it may be used
         * to hold a concatenation of 2 plaintext or ciphertext results.
         * Hence, its size is a double of the normal data size.
         */
        msg_work = new dp_mem_t(1, AES_TESTVEC_MSG_SIZE_MAX * 2,
                                DP_MEM_ALIGN_SPEC,
                                aes_testvec.testvec_params.msg_mem_type(),
                                CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                DP_MEM_ALLOC_NO_FILL);
        msg_work->content_size_set(0);
    }

    ~aes_msg_repr_t()
    {
        if (aes_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (aes_testvec.test_success || !aes_testvec.hw_started) {
                if (msg_work) delete msg_work;
                if (msg_output_expected) delete msg_output_expected;
                if (msg_output_actual) delete msg_output_actual;
                if (msg_input_tmp) delete msg_input_tmp;
                if (msg_input) delete msg_input;
                if (iv) delete iv;
                if (key) delete key;
            }
        }
    }

    friend class aes_testvec_t;

private:
    aes_testvec_t&              aes_testvec;
    dp_mem_t                    *key;
    dp_mem_t                    *iv;
    dp_mem_t                    *msg_input;
    dp_mem_t                    *msg_input_tmp;
    dp_mem_t                    *msg_output_actual;
    dp_mem_t                    *msg_output_expected;
    dp_mem_t                    *msg_work;
    aes_t                       *crypto_aes;

    aes_montecarlo_result_t     mc_key_expected;
    aes_montecarlo_result_t     mc_iv_expected;
    aes_montecarlo_result_t     mc_input_expected;
    aes_montecarlo_result_t     mc_output_expected;
    aes_montecarlo_result_t     mc_key_actual;
    aes_montecarlo_result_t     mc_iv_actual;
    aes_montecarlo_result_t     mc_input_actual;
    aes_montecarlo_result_t     mc_output_actual;
    aes_montecarlo_result_t     mc_output_epoch;

    parser_token_id_t           failed_parse_token;
    uint32_t                    failure_expected: 1,
                                push_failure    : 1,
                                compl_failure   : 1,
                                verify_failure  : 1;
};

} // namespace crypto_aes

#endif   // _CRYPTO_AES_TESTVEC_HPP_

