#ifndef _CRYPTO_ECDSA_HPP_
#define _CRYPTO_ECDSA_HPP_

#include "logger.hpp"
#include "offload_base_params.hpp"
#include "dp_mem.hpp"
#include "acc_ring.hpp"
#include "crypto_asym.hpp"
#include "capri_barco_crypto.hpp"
#include "capri_barco_rings.hpp"
#include "dole_if.hpp"

using namespace dp_mem;
using namespace tests;

namespace crypto_ecdsa {

#define CRYPTO_ECDSA_MSG_ACTUAL_SIZE_MAX    512
#define CRYPTO_ECDSA_DIGEST_PADDED_SIZE_MAX 512
#define CRYPTO_ECDSA_SIG_ACTUAL_SIZE_MAX    512

class ecdsa_t;

/*
 * ECDSA key create types
 */
typedef enum {
    ECDSA_KEY_CREATE_VOID,
    ECDSA_KEY_CREATE_SIGN,
    ECDSA_KEY_CREATE_VERIFY,
    ECDSA_KEY_CREATE_ENCRYPT,
    ECDSA_KEY_CREATE_DECRYPT,
} ecdsa_key_create_type_t;

static inline bool
ecdsa_key_create_type_is_sign(ecdsa_key_create_type_t create_type)
{
    return create_type == ECDSA_KEY_CREATE_SIGN;
}

static inline bool
ecdsa_key_create_type_is_verify(ecdsa_key_create_type_t create_type)
{
    return create_type == ECDSA_KEY_CREATE_VERIFY;
}

static inline bool
ecdsa_key_create_type_is_encrypt(ecdsa_key_create_type_t create_type)
{
    return create_type == ECDSA_KEY_CREATE_ENCRYPT;
}

static inline bool
ecdsa_key_create_type_is_decrypt(ecdsa_key_create_type_t create_type)
{
    return create_type == ECDSA_KEY_CREATE_DECRYPT;
}

/*
 * Emulate named parameters support for ecdsa_t constructor
 */
class ecdsa_params_t
{
public:

    ecdsa_params_t() :
        dma_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        msg_digest_mem_type_(DP_MEM_TYPE_HBM),
        acc_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0),
        key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
        key_idx_shared_(false)
    {
    }

    ecdsa_params_t&
    dma_desc_mem_type(dp_mem_type_t dma_desc_mem_type)
    {
        dma_desc_mem_type_ = dma_desc_mem_type;
        return *this;
    }
    ecdsa_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    ecdsa_params_t&
    msg_digest_mem_type(dp_mem_type_t msg_digest_mem_type)
    {
        msg_digest_mem_type_ = msg_digest_mem_type;
        return *this;
    }
    ecdsa_params_t&
    acc_ring(acc_ring_t *acc_ring)
    {
        acc_ring_ = acc_ring;
        return *this;
    }
    ecdsa_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    ecdsa_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }
    ecdsa_params_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    ecdsa_params_t&
    key_idx_shared(bool key_idx_shared)
    {
        key_idx_shared_ = key_idx_shared;
        return *this;
    }
    ecdsa_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t dma_desc_mem_type(void) { return dma_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t msg_digest_mem_type(void) { return msg_digest_mem_type_; }
    acc_ring_t *acc_ring(void) { return acc_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }
    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    bool key_idx_shared(void) { return key_idx_shared_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    dp_mem_type_t               dma_desc_mem_type_;
    dp_mem_type_t               status_mem_type_;
    dp_mem_type_t               msg_digest_mem_type_;
    acc_ring_t                  *acc_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
    crypto_asym::key_idx_t      key_idx_;       // key_idx if already created
    bool                        key_idx_shared_;
    offload_base_params_t       base_params_;
};

/*
 * ECDSA pre-push params
 */
class ecdsa_pre_push_params_t
{
public:

    ecdsa_pre_push_params_t() :
        key_create_type_(ECDSA_KEY_CREATE_VOID),
        n_(nullptr),
        d_(nullptr),
        e_(nullptr),
        msg_(nullptr)
    {
    }

    ecdsa_pre_push_params_t&
    key_create_type(ecdsa_key_create_type_t key_create_type)
    {
        key_create_type_ = key_create_type;
        return *this;
    }
    ecdsa_pre_push_params_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    ecdsa_pre_push_params_t&
    d(dp_mem_t *d)
    {
        d_ = d;
        return *this;
    }
    ecdsa_pre_push_params_t&
    e(dp_mem_t *e)
    {
        e_ = e;
        return *this;
    }
    ecdsa_pre_push_params_t&
    msg(dp_mem_t *msg)
    {
        msg_ = msg;
        return *this;
    }
    ecdsa_pre_push_params_t&
    hash_algo(string hash_algo)
    {
        hash_algo_.assign(hash_algo);
        return *this;
    }

    ecdsa_key_create_type_t key_create_type(void) { return key_create_type_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d(void) { return d_; }
    dp_mem_t *e(void) { return e_; }
    dp_mem_t *msg(void) { return msg_; }
    const string& hash_algo(void) { return hash_algo_; }

private:
    ecdsa_key_create_type_t     key_create_type_;
    dp_mem_t                    *n_;            // ECDSA modulus
    dp_mem_t                    *d_;            // ECDSA private exponent
    dp_mem_t                    *e_;            // ECDSA public exponent
    dp_mem_t                    *msg_;
    string                      hash_algo_;
};

/*
 * ECDSA push params
 */
class ecdsa_push_params_t
{
public:

    ecdsa_push_params_t() :
        msg_expected_(nullptr),
        msg_actual_(nullptr),
        sig_expected_(nullptr),
        sig_actual_(nullptr),
        failure_expected_(false)
    {
    }

    ecdsa_push_params_t&
    msg_expected(dp_mem_t *msg_expected)
    {
        msg_expected_ = msg_expected;
        return *this;
    }
    ecdsa_push_params_t&
    msg_actual(dp_mem_t *msg_actual)
    {
        msg_actual_ = msg_actual;
        return *this;
    }
    ecdsa_push_params_t&
    sig_expected(dp_mem_t *sig_expected)
    {
        sig_expected_ = sig_expected;
        return *this;
    }
    ecdsa_push_params_t&
    sig_actual(dp_mem_t *sig_actual)
    {
        sig_actual_ = sig_actual;
        return *this;
    }
    ecdsa_push_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }

    dp_mem_t *msg_expected(void) { return msg_expected_; }
    dp_mem_t *msg_actual(void) { return msg_actual_; }
    dp_mem_t *sig_expected(void) { return sig_expected_; }
    dp_mem_t *sig_actual(void) { return sig_actual_; }
    bool failure_expected(void) { return failure_expected_; }

private:
    dp_mem_t                    *msg_expected_;
    dp_mem_t                    *msg_actual_;
    dp_mem_t                    *sig_expected_;
    dp_mem_t                    *sig_actual_;
    bool                        failure_expected_;
};

/*
 * ECDSA hardware signature generation push params
 * with padded msg digest.
 */
class ecdsa_hw_sign_params_t
{
public:

    ecdsa_hw_sign_params_t() :
        hash_input_(nullptr),
        sig_output_(nullptr)
    {
    }

    ecdsa_hw_sign_params_t&
    hash_input(dp_mem_t *hash_input)
    {
        hash_input_ = hash_input;
        return *this;
    }
    ecdsa_hw_sign_params_t&
    sig_output(dp_mem_t *sig_output)
    {
        sig_output_ = sig_output;
        return *this;
    }

    dp_mem_t *hash_input(void) { return hash_input_; }
    dp_mem_t *sig_output(void) { return sig_output_; }

private:
    dp_mem_t                    *hash_input_;
    dp_mem_t                    *sig_output_;
};

/*
 * ECDSA hardware signature verification push params
 */
class ecdsa_hw_verify_params_t
{
public:

    ecdsa_hw_verify_params_t() :
        sig_input_(nullptr),
        hash_input_(nullptr)
    {
    }

    ecdsa_hw_verify_params_t&
    sig_input(dp_mem_t *sig_input)
    {
        sig_input_ = sig_input;
        return *this;
    }
    ecdsa_hw_verify_params_t&
    hash_input(dp_mem_t *hash_input)
    {
        hash_input_ = hash_input;
        return *this;
    }

    dp_mem_t *sig_input(void) { return sig_input_; }
    dp_mem_t *hash_input(void) { return hash_input_; }

private:
    dp_mem_t                    *sig_input_;
    dp_mem_t                    *hash_input_;
};

/*
 * ECDSA hardware encryption push params
 */
class ecdsa_hw_enc_params_t
{
public:

    ecdsa_hw_enc_params_t() :
        plain_input_(nullptr),
        ciphered_output_(nullptr)
    {
    }

    ecdsa_hw_enc_params_t&
    plain_input(dp_mem_t *plain_input)
    {
        plain_input_ = plain_input;
        return *this;
    }
    ecdsa_hw_enc_params_t&
    ciphered_output(dp_mem_t *ciphered_output)
    {
        ciphered_output_ = ciphered_output;
        return *this;
    }

    dp_mem_t *plain_input(void) { return plain_input_; }
    dp_mem_t *ciphered_output(void) { return ciphered_output_; }

private:
    dp_mem_t                    *plain_input_;
    dp_mem_t                    *ciphered_output_;
};

/*
 * ECDSA hardware decryption push params
 */
class ecdsa_hw_dec_params_t
{
public:

    ecdsa_hw_dec_params_t() :
        ciphered_input_(nullptr),
        plain_output_(nullptr)
    {
    }

    ecdsa_hw_dec_params_t&
    ciphered_input(dp_mem_t *ciphered_input)
    {
        ciphered_input_ = ciphered_input;
        return *this;
    }
    ecdsa_hw_dec_params_t&
    plain_output(dp_mem_t *plain_output)
    {
        plain_output_ = plain_output;
        return *this;
    }

    dp_mem_t *ciphered_input(void) { return ciphered_input_; }
    dp_mem_t *plain_output(void) { return plain_output_; }

private:
    dp_mem_t                    *ciphered_input_;
    dp_mem_t                    *plain_output_;
};

/*
 * ECDSA Crypto
 */
class ecdsa_t {

public:
    ecdsa_t(ecdsa_params_t& params);
    ~ecdsa_t();

    bool pre_push(ecdsa_pre_push_params_t& pre_params);
    bool push(ecdsa_push_params_t& push_params);
    bool push(ecdsa_hw_sign_params_t& hw_sign_params);
    bool push(ecdsa_hw_verify_params_t& hw_verify_params);
    bool push(ecdsa_hw_enc_params_t& hw_enc_params);
    bool push(ecdsa_hw_dec_params_t& hw_dec_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);

    crypto_asym::key_idx_t key_idx_get(void) { return key_idx; }

private:

    bool key_create(ecdsa_pre_push_params_t& pre_params);
    void key_destroy(void);
    bool expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual);
    void test_params_report(void);

    ecdsa_params_t              ecdsa_params;
    ecdsa_pre_push_params_t     pre_params;
    ecdsa_push_params_t         push_params;
    ecdsa_hw_sign_params_t      hw_sign_params;
    ecdsa_hw_verify_params_t    hw_verify_params;
    ecdsa_hw_enc_params_t       hw_enc_params;
    ecdsa_hw_dec_params_t       hw_dec_params;

    crypto_asym::dma_desc_pool_t *dma_desc_pool;
    crypto_asym::status_t       *status;
    dp_mem_t                    *digest;
    dp_mem_t                    *digest_padded;
    const dole_if::dole_evp_md_t *evp_md;
    crypto_asym::key_idx_t      key_idx;

    bool                        hw_started;
    bool                        test_success;
};

} // namespace crypto_ecdsa

#endif   // _CRYPTO_ECDSA_HPP_
