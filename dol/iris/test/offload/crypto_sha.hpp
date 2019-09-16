#ifndef _CRYPTO_SHA_HPP_
#define _CRYPTO_SHA_HPP_

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

namespace crypto_sha {

#define CRYPTO_SHA_DIGEST_SIZE_MAX        SHA512_DIGEST_LENGTH

class sha_t;

/*
 * Emulate named parameters support for sha_t constructor
 */
class sha_params_t
{
public:

    sha_params_t() :
        msg_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        doorbell_mem_type_(DP_MEM_TYPE_HBM),
        msg_digest_mem_type_(DP_MEM_TYPE_HBM),
        acc_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    sha_params_t&
    msg_desc_mem_type(dp_mem_type_t msg_desc_mem_type)
    {
        msg_desc_mem_type_ = msg_desc_mem_type;
        return *this;
    }
    sha_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    sha_params_t&
    doorbell_mem_type(dp_mem_type_t doorbell_mem_type)
    {
        doorbell_mem_type_ = doorbell_mem_type;
        return *this;
    }
    sha_params_t&
    msg_digest_mem_type(dp_mem_type_t msg_digest_mem_type)
    {
        msg_digest_mem_type_ = msg_digest_mem_type;
        return *this;
    }
    sha_params_t&
    acc_ring(acc_ring_t *acc_ring)
    {
        acc_ring_ = acc_ring;
        return *this;
    }
    sha_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    sha_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }
    sha_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t msg_desc_mem_type(void) { return msg_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t doorbell_mem_type(void) { return doorbell_mem_type_; }
    dp_mem_type_t msg_digest_mem_type(void) { return msg_digest_mem_type_; }
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
 * SHA pre-push params
 */
class sha_pre_push_params_t
{
public:

    sha_pre_push_params_t() :
        crypto_symm_type_(crypto_symm::CRYPTO_SYMM_TYPE_SHA),
        sha_nbytes_(0)
    {
    }

    sha_pre_push_params_t&
    crypto_symm_type(crypto_symm::crypto_symm_type_t crypto_symm_type)
    {
        crypto_symm_type_ = crypto_symm_type;
        return *this;
    }
    sha_pre_push_params_t&
    sha_nbytes(uint32_t sha_nbytes)
    {
        sha_nbytes_ = sha_nbytes;
        return *this;
    }

    crypto_symm::crypto_symm_type_t crypto_symm_type(void) { return crypto_symm_type_; }
    uint32_t sha_nbytes(void) { return sha_nbytes_; }

private:
    crypto_symm::crypto_symm_type_t crypto_symm_type_;
    uint32_t                    sha_nbytes_;
};

/*
 * SHA push params
 */
class sha_push_params_t
{
public:

    sha_push_params_t() :
        msg_(nullptr),
        md_expected_(nullptr),
        md_actual_(nullptr),
        failure_expected_(false),
        wait_for_completion_(false),
        ret_evp_md_(nullptr)
    {
    }

    sha_push_params_t&
    msg(dp_mem_t *msg)
    {
        msg_ = msg;
        return *this;
    }
    sha_push_params_t&
    md_expected(dp_mem_t *md_expected)
    {
        md_expected_ = md_expected;
        return *this;
    }
    sha_push_params_t&
    md_actual(dp_mem_t *md_actual)
    {
        md_actual_ = md_actual;
        return *this;
    }
    sha_push_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }
    sha_push_params_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }
    sha_push_params_t&
    ret_evp_md(const eng_if::eng_evp_md_t *evp_md)
    {
        ret_evp_md_ = evp_md;
        return *this;
    }

    dp_mem_t *msg(void) { return msg_; }
    dp_mem_t *md_expected(void) { return md_expected_; }
    dp_mem_t *md_actual(void) { return md_actual_; }
    bool failure_expected(void) { return failure_expected_; }
    bool wait_for_completion(void) { return wait_for_completion_; }
    const eng_if::eng_evp_md_t *ret_evp_md(void) { return ret_evp_md_; }

private:
    dp_mem_t                    *msg_;
    dp_mem_t                    *md_expected_;
    dp_mem_t                    *md_actual_;
    bool                        failure_expected_;
    bool                        wait_for_completion_;
    const eng_if::eng_evp_md_t  *ret_evp_md_;
};

/*
 * SHA HW init params
 */
class sha_hw_init_params_t
{
public:

    sha_hw_init_params_t() {}

private:
};

/*
 * SHA HW update params
 */
class sha_hw_update_params_t
{
public:

    sha_hw_update_params_t() :
        msg_input_(nullptr)
    {
    }

    sha_hw_update_params_t&
    msg_input(dp_mem_t *msg_input)
    {
        msg_input_ = msg_input;
        return *this;
    }

    dp_mem_t *msg_input(void) { return msg_input_; }

private:
    dp_mem_t                    *msg_input_;
};

/*
 * SHA HW final params
 */
class sha_hw_final_params_t
{
public:

    sha_hw_final_params_t() :
        hash_output_(nullptr)
    {
    }

    sha_hw_final_params_t&
    hash_output(dp_mem_t *hash_output)
    {
        hash_output_ = hash_output;
        return *this;
    }

    dp_mem_t *hash_output(void) { return hash_output_; }

private:
    dp_mem_t                    *hash_output_;
};

/*
 * SHA HW cleanup params
 */
class sha_hw_cleanup_params_t
{
public:

    sha_hw_cleanup_params_t() {}

private:
};

/*
 * Secure Hash Algorithm
 */
class sha_t {

public:
    sha_t(sha_params_t& params);
    ~sha_t();

    bool pre_push(sha_pre_push_params_t& pre_params);
    bool push(sha_push_params_t& push_params);
    bool push(sha_hw_init_params_t& hw_init_params);
    bool push(sha_hw_update_params_t& hw_update_params);
    bool push(sha_hw_final_params_t& hw_final_params);
    bool push(sha_hw_cleanup_params_t& hw_cleanup_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    bool expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual);
private:

    void test_params_report(void);

    sha_params_t                sha_params;
    sha_pre_push_params_t       pre_params;
    sha_push_params_t           push_params;

    string                      sha_algo;
    crypto_symm::msg_desc_pool_t *msg_desc_pool;
    crypto_symm::status_t       *status;
    crypto_symm::doorbell_t     *doorbell;
    const eng_if::eng_evp_md_t  *evp_md;

    vector<dp_mem_t *>          msg_input_vec;

    bool                        hw_started;
    bool                        test_success;
};


string sha_algo_find(uint32_t sha_nbytes);
uint32_t sha_nbytes_find(const string& sha_algo);
string sha3_algo_find(uint32_t sha_nbytes);
uint32_t sha3_nbytes_find(const string& sha_algo);

/*
 * Access methods for PSE Openssl engine
 */
extern "C" {
extern const PSE_OFFLOAD_MD_METHOD pse_md_offload_method;
}

} // namespace crypto_sha

#endif   // _CRYPTO_SHA_HPP_
