#ifndef _CRYPTO_RSA_TESTVEC_HPP_
#define _CRYPTO_RSA_TESTVEC_HPP_

#include <stdint.h>
#include <string>
#include <memory>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <openssl/ssl.h>
#include "logger.hpp"
#include "offload_base_params.hpp"
#include "acc_ring.hpp"
#include "testvec_parser.hpp"
#include "crypto_rsa.hpp"

using namespace tests;

#define CRYPTO_RSA_SIG_ACTUAL_SIZE_MAX    512

namespace crypto_rsa {

class rsa_key_repr_t;
class rsa_msg_repr_t;
class rsa_testvec_t;

/*
 * Emulate named parameters support for rsa_testvec_t constructor
 */
class rsa_testvec_params_t
{
public:

    rsa_testvec_params_t() :
        e_mem_type_(DP_MEM_TYPE_HBM),
        n_mem_type_(DP_MEM_TYPE_HBM),
        d_mem_type_(DP_MEM_TYPE_HBM),
        msg_mem_type_(DP_MEM_TYPE_HBM),
        sig_mem_type_(DP_MEM_TYPE_HBM)
    {
    }

    rsa_testvec_params_t&
    e_mem_type(dp_mem_type_t e_mem_type)
    {
        e_mem_type_ = e_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    n_mem_type(dp_mem_type_t n_mem_type)
    {
        n_mem_type_ = n_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    d_mem_type(dp_mem_type_t d_mem_type)
    {
        d_mem_type_ = d_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    msg_mem_type(dp_mem_type_t msg_mem_type)
    {
        msg_mem_type_ = msg_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    sig_mem_type(dp_mem_type_t sig_mem_type)
    {
        sig_mem_type_ = sig_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t e_mem_type(void) { return e_mem_type_; }
    dp_mem_type_t n_mem_type(void) { return n_mem_type_; }
    dp_mem_type_t d_mem_type(void) { return d_mem_type_; }
    dp_mem_type_t msg_mem_type(void) { return msg_mem_type_; }
    dp_mem_type_t sig_mem_type(void) { return sig_mem_type_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    dp_mem_type_t               e_mem_type_;
    dp_mem_type_t               n_mem_type_;
    dp_mem_type_t               d_mem_type_;
    dp_mem_type_t               msg_mem_type_;
    dp_mem_type_t               sig_mem_type_;
    offload_base_params_t       base_params_;

};

/*
 * Emulate named parameters support for rsa_testvec_t pre_push
 */
class rsa_testvec_pre_push_params_t
{
public:

    rsa_testvec_pre_push_params_t() {}

    rsa_testvec_pre_push_params_t&
    testvec_fname(string testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }

    string& testvec_fname(void) { return testvec_fname_; }

private:
    string                      testvec_fname_;
};

/*
 * Emulate named parameters support for rsa_testvec_t push
 */
class rsa_testvec_push_params_t
{
public:

    rsa_testvec_push_params_t() :
        rsa_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    rsa_testvec_push_params_t&
    rsa_ring(acc_ring_t *rsa_ring)
    {
        rsa_ring_ = rsa_ring;
        return *this;
    }
    rsa_testvec_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    rsa_testvec_push_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }

    acc_ring_t *rsa_ring(void) { return rsa_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }

private:
    acc_ring_t                  *rsa_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
};

/*
 * Crypto RSA test vector
 */
class rsa_testvec_t {

public:
    rsa_testvec_t(rsa_testvec_params_t& params);
    ~rsa_testvec_t();

    friend class rsa_key_repr_t;
    friend class rsa_msg_repr_t;

    bool pre_push(rsa_testvec_pre_push_params_t& pre_params);
    bool push(rsa_testvec_push_params_t& push_params);
    bool post_push(void);
    bool fast_verify(void);
    bool full_verify(void);

private:
    rsa_testvec_params_t        testvec_params;
    rsa_testvec_push_params_t   push_params;
    testvec_parser_t            *testvec_parser;
    vector<shared_ptr<rsa_key_repr_t>> key_repr_vec;

    uint32_t                    num_test_failures;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Crypto RSA key representative
 */
class rsa_key_repr_t {

public:
    rsa_key_repr_t(rsa_testvec_t& rsa_testvec,
                   u_long modulus_bytes_len) :
        rsa_testvec(rsa_testvec),
        modulus_bytes_len(modulus_bytes_len)
    {
        assert(modulus_bytes_len);
        e = new dp_mem_t(1, modulus_bytes_len,
                         DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.e_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        n = new dp_mem_t(1, modulus_bytes_len,
                         DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.n_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        d = new dp_mem_t(1, modulus_bytes_len,
                         DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.d_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
    }

    ~rsa_key_repr_t()
    {
        if (rsa_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (rsa_testvec.test_success || !rsa_testvec.hw_started) {
                if (e) delete e;
                if (n) delete n;
                if (d) delete d;
            }
        }
    }

    friend class rsa_testvec_t;

private:
    rsa_testvec_t&              rsa_testvec;
    u_long                      modulus_bytes_len;
    dp_mem_t                    *e;             // RSA public exponent
    dp_mem_t                    *n;             // RSA modulus
    dp_mem_t                    *d;             // RSA private exponent
    vector<shared_ptr<rsa_msg_repr_t>> msg_repr_vec;
};

/*
 * Crypto RSA message representative
 */
class rsa_msg_repr_t {

public:
    rsa_msg_repr_t(rsa_testvec_t& rsa_testvec,
                   u_long modulus_bytes_len) :
        rsa_testvec(rsa_testvec),
        modulus_bytes_len(modulus_bytes_len),
        crypto_rsa(nullptr)
    {
        assert(modulus_bytes_len);
        msg = new dp_mem_t(1, modulus_bytes_len,
                           DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.msg_mem_type(),
                           0, DP_MEM_ALLOC_NO_FILL);
        sig_expected = new dp_mem_t(1, modulus_bytes_len,
                           DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.sig_mem_type(),
                           0, DP_MEM_ALLOC_FILL_ZERO);
        sig_actual = new dp_mem_t(1, CRYPTO_RSA_SIG_ACTUAL_SIZE_MAX,
                         DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.sig_mem_type(),
                         0, DP_MEM_ALLOC_FILL_ZERO);
    }

    ~rsa_msg_repr_t()
    {
        if (rsa_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (rsa_testvec.test_success || !rsa_testvec.hw_started) {
                if (crypto_rsa) delete crypto_rsa;
                if (sig_actual) delete sig_actual;
                if (sig_expected) delete sig_expected;
                if (msg) delete msg;
            }
        }
    }

    friend class rsa_testvec_t;

private:
    rsa_testvec_t&              rsa_testvec;
    u_long                      modulus_bytes_len;
    string                      sha_algo;
    dp_mem_t                    *msg;
    dp_mem_t                    *sig_expected;
    dp_mem_t                    *sig_actual;
    rsa_t                       *crypto_rsa;
};

} // namespace crypto_rsa

#endif   // _CRYPTO_RSA_TESTVEC_HPP_

