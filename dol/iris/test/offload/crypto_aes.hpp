#ifndef _CRYPTO_AES_HPP_
#define _CRYPTO_AES_HPP_

#include "logger.hpp"
#include "offload_base_params.hpp"
#include "dp_mem.hpp"
#include "acc_ring.hpp"
#include "crypto_symm.hpp"
#include "capri_barco_crypto.hpp"
#include "capri_barco_rings.hpp"
#include "nic/sdk/include/sdk/pse_intf.h"
#include "eng_if.hpp"

using namespace dp_mem;
using namespace tests;

namespace crypto_aes {

#define CRYPTO_AES_128_KEY_SIZE         16
#define CRYPTO_AES_192_KEY_SIZE         24
#define CRYPTO_AES_256_KEY_SIZE         32
#define CRYPTO_AES_KEY_PART_SIZE_MAX    CRYPTO_AES_256_KEY_SIZE
#define CRYPTO_AES_KEY_NUM_PARTS_MAX    2

#define CRYPTO_AES_IV_SIZE_MAX          EVP_MAX_IV_LENGTH
#define CRYPTO_AES_KEY_SIZE_MAX         EVP_MAX_KEY_LENGTH
#define CRYPTO_AES_DATA_BLOCK_SIZE      AES_BLOCK_SIZE

typedef enum {
    CRYPTO_AES_NOT_MONTECARLO,          // Not a Montecarlo test
    CRYPTO_AES_MONTECARLO_ECB,          // Montecarlo Electronic Codebook
    CRYPTO_AES_MONTECARLO_CBC,          // Montecarlo Cipher Block Chaining
    CRYPTO_AES_MONTECARLO_OFB,          // Montecarlo Output Feedback
    CRYPTO_AES_MONTECARLO_CFB1,         // Montecarlo Cipher Feedback
    CRYPTO_AES_MONTECARLO_CFB8,
    CRYPTO_AES_MONTECARLO_CFB128,
} crypto_aes_montecarlo_t;

class aes_t;

/*
 * Emulate named parameters support for aes_t constructor
 */
class aes_params_t
{
public:

    aes_params_t() :
        msg_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        doorbell_mem_type_(DP_MEM_TYPE_HBM),
        acc_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    aes_params_t&
    msg_desc_mem_type(dp_mem_type_t msg_desc_mem_type)
    {
        msg_desc_mem_type_ = msg_desc_mem_type;
        return *this;
    }
    aes_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    aes_params_t&
    doorbell_mem_type(dp_mem_type_t doorbell_mem_type)
    {
        doorbell_mem_type_ = doorbell_mem_type;
        return *this;
    }
    aes_params_t&
    acc_ring(acc_ring_t *acc_ring)
    {
        acc_ring_ = acc_ring;
        return *this;
    }
    aes_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    aes_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }
    aes_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t msg_desc_mem_type(void) { return msg_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t doorbell_mem_type(void) { return doorbell_mem_type_; }
    acc_ring_t *acc_ring(void) { return acc_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    dp_mem_type_t               msg_desc_mem_type_;
    dp_mem_type_t               status_mem_type_;
    dp_mem_type_t               doorbell_mem_type_;
    dp_mem_type_t               msg_digest_mem_type_;
    acc_ring_t                  *acc_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
    offload_base_params_t       base_params_;
};

/*
 * AES pre-push params
 */
class aes_pre_push_params_t
{
public:

    aes_pre_push_params_t() :
        crypto_symm_type_(crypto_symm::CRYPTO_SYMM_TYPE_AES_CBC),
        op_(crypto_symm::CRYPTO_SYMM_OP_ENCRYPT),
        key_(nullptr),
        iv_(nullptr),
        key_idx_(CRYPTO_SYMM_KEY_IDX_INVALID),
        key_idx_shared_(false)
    {
    }

    aes_pre_push_params_t&
    crypto_symm_type(crypto_symm::crypto_symm_type_t crypto_symm_type)
    {
        crypto_symm_type_ = crypto_symm_type;
        return *this;
    }
    aes_pre_push_params_t&
    op(crypto_symm::crypto_symm_op_t op)
    {
        op_ = op;
        return *this;
    }
    aes_pre_push_params_t&
    key(dp_mem_t *key)
    {
        key_ = key;
        return *this;
    }
    aes_pre_push_params_t&
    iv(dp_mem_t *iv)
    {
        iv_ = iv;
        return *this;
    }
    aes_pre_push_params_t&
    key_idx(crypto_symm::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    aes_pre_push_params_t&
    key_idx_shared(bool key_idx_shared)
    {
        key_idx_shared_ = key_idx_shared;
        return *this;
    }

    crypto_symm::crypto_symm_type_t crypto_symm_type(void) { return crypto_symm_type_; }
    crypto_symm::crypto_symm_op_t op(void) { return op_; }
    dp_mem_t *key(void) { return key_; }
    dp_mem_t *iv(void) { return iv_; }
    crypto_symm::key_idx_t key_idx(void) { return key_idx_; }
    bool key_idx_shared(void) { return key_idx_shared_; }

private:
    crypto_symm::crypto_symm_type_t crypto_symm_type_;
    crypto_symm::crypto_symm_op_t op_;
    dp_mem_t                    *key_;
    dp_mem_t                    *iv_;
    crypto_symm::key_idx_t      key_idx_;       // key_idx if already created
    bool                        key_idx_shared_;
};

/*
 * AES push params
 */
class aes_push_params_t
{
public:

    aes_push_params_t() :
        msg_input_(nullptr),
        msg_output_actual_(nullptr),
        msg_output_expected_(nullptr),
        failure_expected_(false),
        wait_for_completion_(false)
    {
    }

    aes_push_params_t&
    msg_input(dp_mem_t *msg_input)
    {
        msg_input_ = msg_input;
        return *this;
    }
    aes_push_params_t&
    msg_output_actual(dp_mem_t *msg_output_actual)
    {
        msg_output_actual_ = msg_output_actual;
        return *this;
    }
    aes_push_params_t&
    msg_output_expected(dp_mem_t *msg_output_expected)
    {
        msg_output_expected_ = msg_output_expected;
        return *this;
    }
    aes_push_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }
    aes_push_params_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }

    dp_mem_t *msg_input(void) { return msg_input_; }
    dp_mem_t *msg_output_actual(void) { return msg_output_actual_; }
    dp_mem_t *msg_output_expected(void) { return msg_output_expected_; }
    bool failure_expected(void) { return failure_expected_; }
    bool wait_for_completion(void) { return wait_for_completion_; }

private:
    dp_mem_t                    *msg_input_;
    dp_mem_t                    *msg_output_actual_;
    dp_mem_t                    *msg_output_expected_;
    bool                        failure_expected_;
    bool                        wait_for_completion_;
};

/*
 * AES HW init params
 */
class aes_hw_init_params_t
{
public:

    aes_hw_init_params_t() :
        key_(nullptr)
    {
    }

    aes_hw_init_params_t&
    key(dp_mem_t *key)
    {
        key_ = key;
        return *this;
    }

    dp_mem_t *key(void) { return key_; }

private:
    dp_mem_t                    *key_;
};

/*
 * AES HW cipher params
 */
class aes_hw_cipher_params_t
{
public:

    aes_hw_cipher_params_t() :
        key_idx_(CRYPTO_SYMM_KEY_IDX_INVALID),
        msg_input_(nullptr),
        msg_output_(nullptr)
    {
    }

    aes_hw_cipher_params_t&
    key_idx(crypto_symm::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    aes_hw_cipher_params_t&
    msg_input(dp_mem_t *msg_input)
    {
        msg_input_ = msg_input;
        return *this;
    }
    aes_hw_cipher_params_t&
    msg_output(dp_mem_t *msg_output)
    {
        msg_output_ = msg_output;
        return *this;
    }

    crypto_symm::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *msg_input(void) { return msg_input_; }
    dp_mem_t *msg_output(void) { return msg_output_; }

private:
    crypto_symm::key_idx_t      key_idx_;
    dp_mem_t                    *msg_input_;
    dp_mem_t                    *msg_output_;
};

/*
 * AES HW cleanup params
 */
class aes_hw_cleanup_params_t
{
public:

    aes_hw_cleanup_params_t() {}

private:
};

/*
 * Advanced Encryption Standard
 */
class aes_t {

public:
    aes_t(aes_params_t& params);
    ~aes_t();

    bool pre_push(aes_pre_push_params_t& pre_params);
    bool push(aes_push_params_t& push_params);
    bool push(aes_hw_init_params_t& hw_init_params);
    bool push(aes_hw_cipher_params_t& hw_cipher_params);
    bool push(aes_hw_cleanup_params_t& hw_cleanup_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    bool expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual);
    crypto_symm::key_idx_t key_idx_get(void) { return key_idx; }
    void key_destroy(bool force_destroy);

private:

    bool key_create(aes_hw_init_params_t& hw_init_params);
    void test_params_report(void);

    aes_params_t                aes_params;
    aes_pre_push_params_t       pre_params;
    aes_push_params_t           push_params;

    string                      aes_algo;
    crypto_symm::msg_desc_pool_t *msg_input_desc_pool;
    crypto_symm::msg_desc_pool_t *msg_output_desc_pool;
    crypto_symm::status_t       *status;
    crypto_symm::doorbell_t     *doorbell;
    eng_if::eng_evp_cipher_ctx_t *evp_cipher_ctx;

    vector<dp_mem_t *>          msg_input_vec;
    vector<dp_mem_t *>          msg_output_vec;

    crypto_symm::key_idx_t      key_idx;
    bool                        hw_started;
    bool                        test_success;
};


/*
 * Access methods for PSE Openssl engine
 */
extern "C" {
extern const PSE_OFFLOAD_CIPHER_METHOD pse_cipher_offload_method;
}

} // namespace crypto_aes

#endif   // _CRYPTO_AES_HPP_
