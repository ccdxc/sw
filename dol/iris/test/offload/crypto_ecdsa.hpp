#ifndef _CRYPTO_ECDSA_HPP_
#define _CRYPTO_ECDSA_HPP_

#include "logger.hpp"
#include "offload_base_params.hpp"
#include "dp_mem.hpp"
#include "acc_ring.hpp"
#include "crypto_asym.hpp"
#include "capri_barco_crypto.hpp"
#include "capri_barco_rings.hpp"
#include "eng_if.hpp"

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

/*
 * HW expects expanded length for certain curve P sizes
 */
static inline uint32_t
ecdsa_expanded_len(uint32_t P_bytes_len)
{
    /*
     * Anything less than P-256 is to be expanded to P-256
     */
#define ECDSA_P_(bits_len)  ((bits_len) / BITS_PER_BYTE)

    if (P_bytes_len <= ECDSA_P_(256)) {
        return ECDSA_P_(256);
    }

    /*
     * After that, expand to multiples of P-512
     */
    return ((P_bytes_len + (ECDSA_P_(512) - 1)) / ECDSA_P_(512)) * ECDSA_P_(512);
}

/*
 * Emulate named parameters support for ecdsa_t constructor
 */
class ecdsa_params_t
{
public:

    ecdsa_params_t() :
        P_bytes_len_(0),
        P_expanded_len_(0),
        dma_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        msg_digest_mem_type_(DP_MEM_TYPE_HBM),
        acc_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    ecdsa_params_t&
    P_bytes_len(u_long P_bytes_len)
    {
        P_bytes_len_ = P_bytes_len;
        return *this;
    }
    ecdsa_params_t&
    P_expanded_len(u_long P_expanded_len)
    {
        P_expanded_len_ = P_expanded_len;
        return *this;
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
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    u_long P_bytes_len(void) { return P_bytes_len_; }
    u_long P_expanded_len(void) { return P_expanded_len_; }
    dp_mem_type_t dma_desc_mem_type(void) { return dma_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t msg_digest_mem_type(void) { return msg_digest_mem_type_; }
    acc_ring_t *acc_ring(void) { return acc_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    u_long                      P_bytes_len_;
    u_long                      P_expanded_len_;
    dp_mem_type_t               dma_desc_mem_type_;
    dp_mem_type_t               status_mem_type_;
    dp_mem_type_t               msg_digest_mem_type_;
    acc_ring_t                  *acc_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
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
        curve_nid_(NID_undef),
        domain_vec_(nullptr),
        d_(nullptr),
        msg_(nullptr)
    {
    }

    ecdsa_pre_push_params_t&
    curve_nid(int curve_nid)
    {
        curve_nid_ = curve_nid;
        return *this;
    }
    ecdsa_pre_push_params_t&
    domain_vec(dp_mem_t *domain_vec)
    {
        domain_vec_ = domain_vec;
        return *this;
    }
    ecdsa_pre_push_params_t&
    key_create_type(ecdsa_key_create_type_t key_create_type)
    {
        key_create_type_ = key_create_type;
        return *this;
    }
    ecdsa_pre_push_params_t&
    d(dp_mem_t *d)
    {
        d_ = d;
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
    int curve_nid(void) { return curve_nid_; }
    dp_mem_t *domain_vec(void) { return domain_vec_; }
    dp_mem_t *d(void) { return d_; }
    dp_mem_t *msg(void) { return msg_; }
    const string& hash_algo(void) { return hash_algo_; }

private:
    ecdsa_key_create_type_t     key_create_type_;
    int                         curve_nid_;
    dp_mem_t                    *domain_vec_;
    dp_mem_t                    *d_;            // ECDSA private key
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
        k_random_(nullptr),
        sig_expected_vec_(nullptr),
        r_expected_(nullptr),
        s_expected_(nullptr),
        sig_actual_vec_(nullptr),
        r_actual_(nullptr),
        s_actual_(nullptr),
        failure_expected_(false)
    {
    }

    ecdsa_push_params_t&
    k_random(dp_mem_t *k_random)
    {
        k_random_ = k_random;
        return *this;
    }
    ecdsa_push_params_t&
    sig_expected_vec(dp_mem_t *sig_expected_vec)
    {
        sig_expected_vec_ = sig_expected_vec;
        return *this;
    }
    ecdsa_push_params_t&
    r_expected(dp_mem_t *r_expected)
    {
        r_expected_ = r_expected;
        return *this;
    }
    ecdsa_push_params_t&
    s_expected(dp_mem_t *s_expected)
    {
        s_expected_ = s_expected;
        return *this;
    }
    ecdsa_push_params_t&
    sig_actual_vec(dp_mem_t *sig_actual_vec)
    {
        sig_actual_vec_ = sig_actual_vec;
        return *this;
    }
    ecdsa_push_params_t&
    r_actual(dp_mem_t *r_actual)
    {
        r_actual_ = r_actual;
        return *this;
    }
    ecdsa_push_params_t&
    s_actual(dp_mem_t *s_actual)
    {
        s_actual_ = s_actual;
        return *this;
    }
    ecdsa_push_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }

    dp_mem_t *k_random(void) { return k_random_; }
    dp_mem_t *sig_expected_vec(void) { return sig_expected_vec_; }
    dp_mem_t *r_expected(void) { return r_expected_; }
    dp_mem_t *s_expected(void) { return s_expected_; }
    dp_mem_t *sig_actual_vec(void) { return sig_actual_vec_; }
    dp_mem_t *r_actual(void) { return r_actual_; }
    dp_mem_t *s_actual(void) { return s_actual_; }
    bool failure_expected(void) { return failure_expected_; }

private:
    dp_mem_t                    *k_random_;
    dp_mem_t                    *sig_expected_vec_;
    dp_mem_t                    *r_expected_;
    dp_mem_t                    *s_expected_;
    dp_mem_t                    *sig_actual_vec_;
    dp_mem_t                    *r_actual_;
    dp_mem_t                    *s_actual_;
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
        k_random_(nullptr),
        hash_input_(nullptr),
        sig_output_vec_(nullptr)
    {
    }

    ecdsa_hw_sign_params_t&
    k_random(dp_mem_t *k_random)
    {
        k_random_ = k_random;
        return *this;
    }
    ecdsa_hw_sign_params_t&
    hash_input(dp_mem_t *hash_input)
    {
        hash_input_ = hash_input;
        return *this;
    }
    ecdsa_hw_sign_params_t&
    sig_output_vec(dp_mem_t *sig_output_vec)
    {
        sig_output_vec_ = sig_output_vec;
        return *this;
    }

    dp_mem_t *k_random(void) { return k_random_; }
    dp_mem_t *hash_input(void) { return hash_input_; }
    dp_mem_t *sig_output_vec(void) { return sig_output_vec_; }

private:
    dp_mem_t                    *k_random_;
    dp_mem_t                    *hash_input_;
    dp_mem_t                    *sig_output_vec_;
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

    crypto_asym::dma_desc_pool_t *dma_desc_pool;
    crypto_asym::status_t       *status;
    dp_mem_t                    *digest;
    const eng_if::eng_evp_md_t  *evp_md;
    crypto_asym::key_idx_t      key_idx;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Access methods for PSE Openssl engine
 */
extern "C" {
extern const PSE_EC_OFFLOAD_METHOD pse_ec_offload_method;
}

} // namespace crypto_ecdsa

#endif   // _CRYPTO_ECDSA_HPP_
