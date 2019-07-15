#ifndef _CRYPTO_RSA_HPP_
#define _CRYPTO_RSA_HPP_

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

namespace crypto_rsa {

#define RSA_DMA_DESC_MAX        8

class rsa_t;

/*
 * RSA key create types
 */
typedef enum {
    RSA_KEY_CREATE_VOID,
    RSA_KEY_CREATE_PRIV_SIG_GEN,
    RSA_KEY_CREATE_PRIV_SIG_VERIFY,
    RSA_KEY_CREATE_PUB_SIG_GEN,
    RSA_KEY_CREATE_PUB_SIG_VERIFY,
} rsa_key_create_type_t;

/*
 * Emulate named parameters support for rsa_t constructor
 */
class rsa_params_t
{
public:

    rsa_params_t() :
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

    rsa_params_t&
    dma_desc_mem_type(dp_mem_type_t dma_desc_mem_type)
    {
        dma_desc_mem_type_ = dma_desc_mem_type;
        return *this;
    }
    rsa_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    rsa_params_t&
    msg_digest_mem_type(dp_mem_type_t msg_digest_mem_type)
    {
        msg_digest_mem_type_ = msg_digest_mem_type;
        return *this;
    }
    rsa_params_t&
    acc_ring(acc_ring_t *acc_ring)
    {
        acc_ring_ = acc_ring;
        return *this;
    }
    rsa_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    rsa_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }
    rsa_params_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    rsa_params_t&
    key_idx_shared(bool key_idx_shared)
    {
        key_idx_shared_ = key_idx_shared;
        return *this;
    }
    rsa_params_t&
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
 * RSA signature creation (generation or verification) pre-push params
 */
class rsa_sig_pre_push_params_t
{
public:

    rsa_sig_pre_push_params_t() :
        key_create_type_(RSA_KEY_CREATE_VOID),
        n_(nullptr),
        d_(nullptr),
        e_(nullptr),
        msg_(nullptr)
    {
    }

    rsa_sig_pre_push_params_t&
    key_create_type(rsa_key_create_type_t key_create_type)
    {
        key_create_type_ = key_create_type;
        return *this;
    }
    rsa_sig_pre_push_params_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    rsa_sig_pre_push_params_t&
    d(dp_mem_t *d)
    {
        d_ = d;
        return *this;
    }
    rsa_sig_pre_push_params_t&
    e(dp_mem_t *e)
    {
        e_ = e;
        return *this;
    }
    rsa_sig_pre_push_params_t&
    msg(dp_mem_t *msg)
    {
        msg_ = msg;
        return *this;
    }
    rsa_sig_pre_push_params_t&
    hash_algo(string hash_algo)
    {
        hash_algo_.assign(hash_algo);
        return *this;
    }

    rsa_key_create_type_t key_create_type(void) { return key_create_type_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d(void) { return d_; }
    dp_mem_t *e(void) { return e_; }
    dp_mem_t *msg(void) { return msg_; }
    const string& hash_algo(void) { return hash_algo_; }

private:
    rsa_key_create_type_t       key_create_type_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_;            // RSA private exponent
    dp_mem_t                    *e_;            // RSA public exponent
    dp_mem_t                    *msg_;
    string                      hash_algo_;
};

/*
 * RSA signature creation (generation or verification) push params
 */
class rsa_sig_push_params_t
{
public:

    rsa_sig_push_params_t() :
        sig_expected_(nullptr),
        sig_actual_(nullptr)
    {
    }

    rsa_sig_push_params_t&
    sig_expected(dp_mem_t *sig_expected)
    {
        sig_expected_ = sig_expected;
        return *this;
    }
    rsa_sig_push_params_t&
    sig_actual(dp_mem_t *sig_actual)
    {
        sig_actual_ = sig_actual;
        return *this;
    }

    dp_mem_t *sig_expected(void) { return sig_expected_; }
    dp_mem_t *sig_actual(void) { return sig_actual_; }

private:
    dp_mem_t                    *sig_expected_;
    dp_mem_t                    *sig_actual_;
};

/*
 * RSA signature creation (generation or verification) push params
 * with padded msg digest.
 */
class rsa_sig_padded_push_params_t
{
public:

    rsa_sig_padded_push_params_t() :
        digest_padded_(nullptr),
        sig_actual_(nullptr)
    {
    }

    rsa_sig_padded_push_params_t&
    digest_padded(dp_mem_t *digest_padded)
    {
        digest_padded_ = digest_padded;
        return *this;
    }
    rsa_sig_padded_push_params_t&
    sig_actual(dp_mem_t *sig_actual)
    {
        sig_actual_ = sig_actual;
        return *this;
    }

    dp_mem_t *digest_padded(void) { return digest_padded_; }
    dp_mem_t *sig_actual(void) { return sig_actual_; }

private:
    dp_mem_t                    *digest_padded_;
    dp_mem_t                    *sig_actual_;
};

/*
 * RSA Crypto
 */
class rsa_t {

public:
    rsa_t(rsa_params_t& params);
    ~rsa_t();

    bool pre_push(rsa_sig_pre_push_params_t& sig_pre_params);
    bool push(rsa_sig_push_params_t& sig_push_params);
    bool push(rsa_sig_padded_push_params_t& sig_padded_params);
    bool post_push(void);
    bool fast_verify(void);
    bool full_verify(void);

    crypto_asym::key_idx_t key_idx_get(void) { return key_idx; }

private:

    bool key_create(rsa_sig_pre_push_params_t& sig_pre_params);
    void key_destroy(void);

    rsa_params_t                rsa_params;
    rsa_sig_pre_push_params_t   sig_pre_params;
    rsa_sig_push_params_t       sig_push_params;
    rsa_sig_padded_push_params_t sig_padded_params;

    crypto_asym::dma_desc_pool_t *dma_desc_pool;
    dp_mem_t                    *asym_status;
    dp_mem_t                    *msg_digest;
    const dole_if::dole_evp_md_t *evp_md;
    crypto_asym::key_idx_t      key_idx;

    bool                        hw_started;
    bool                        test_success;
};

} // namespace crypto_rsa

#endif   // _CRYPTO_RSA_HPP_
