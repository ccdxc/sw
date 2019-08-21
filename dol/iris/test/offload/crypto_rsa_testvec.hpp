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
        key_create_type_(RSA_KEY_CREATE_VOID),
        pad_mode_(RSA_NO_PADDING),
        e_mem_type_(DP_MEM_TYPE_HBM),
        n_mem_type_(DP_MEM_TYPE_HBM),
        d_mem_type_(DP_MEM_TYPE_HBM),
        msg_mem_type_(DP_MEM_TYPE_HBM),
        sig_mem_type_(DP_MEM_TYPE_HBM),
        dma_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM)
    {
    }

    rsa_testvec_params_t&
    key_create_type(rsa_key_create_type_t key_create_type)
    {
        key_create_type_ = key_create_type;
        return *this;
    }
    rsa_testvec_params_t&
    pad_mode(int pad_mode)
    {
        pad_mode_ = pad_mode;
        return *this;
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
    dma_desc_mem_type(dp_mem_type_t dma_desc_mem_type)
    {
        dma_desc_mem_type_ = dma_desc_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    rsa_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    rsa_key_create_type_t key_create_type(void) { return key_create_type_; }
    int pad_mode(void) { return pad_mode_; }
    dp_mem_type_t e_mem_type(void) { return e_mem_type_; }
    dp_mem_type_t n_mem_type(void) { return n_mem_type_; }
    dp_mem_type_t d_mem_type(void) { return d_mem_type_; }
    dp_mem_type_t msg_mem_type(void) { return msg_mem_type_; }
    dp_mem_type_t sig_mem_type(void) { return sig_mem_type_; }
    dp_mem_type_t dma_desc_mem_type(void) { return dma_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    rsa_key_create_type_t       key_create_type_;
    int                         pad_mode_;
    dp_mem_type_t               e_mem_type_;
    dp_mem_type_t               n_mem_type_;
    dp_mem_type_t               d_mem_type_;
    dp_mem_type_t               msg_mem_type_;
    dp_mem_type_t               sig_mem_type_;
    dp_mem_type_t               dma_desc_mem_type_;
    dp_mem_type_t               status_mem_type_;
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
    scripts_dir(const string& scripts_dir)
    {
        scripts_dir_.assign(scripts_dir);
        return *this;
    }
    rsa_testvec_pre_push_params_t&
    testvec_fname(const string& testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }

    string& scripts_dir(void) { return scripts_dir_; }
    string& testvec_fname(void) { return testvec_fname_; }

private:
    string                      scripts_dir_;
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
    bool completion_check(void);
    bool full_verify(void);
    void rsp_file_output(const string& mem_type_str);

private:
    rsa_testvec_params_t        testvec_params;
    rsa_testvec_pre_push_params_t pre_params;
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
        modulus_bytes_len(modulus_bytes_len),
        failed_parse_token(PARSE_TOKEN_ID_VOID)
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
    parser_token_id_t           failed_parse_token;
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
        crypto_rsa(nullptr),
        failed_parse_token(PARSE_TOKEN_ID_VOID),
        failure_expected(false),
        push_failure(false),
        compl_failure(false),
        verify_failure(false)
    {
        assert(modulus_bytes_len);

        /*
         * Normally we'd allocate a dp_mem vector here to achieve memory 
         * contiguity but if the underlying mem_type is HBM, the corresponding
         * allocation library may pad out the length and defeat contiguity.
         * Hence, we have to resort to allocating double size so that
         * contiguity can still be obtained with fragment_find().
         */
        msg_vec = new dp_mem_t(1, modulus_bytes_len * 2,
                               DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.msg_mem_type(),
                               0, DP_MEM_ALLOC_FILL_ZERO);
        msg_expected = msg_vec->fragment_find(0, modulus_bytes_len);
        msg_actual = msg_vec->fragment_find(modulus_bytes_len, modulus_bytes_len);

        // ensure the 2 fragments are contiguous in memory
        assert((msg_expected->pa() + modulus_bytes_len) == msg_actual->pa());

        sig_vec = new dp_mem_t(1, modulus_bytes_len * 2,
                               DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.sig_mem_type(),
                               0, DP_MEM_ALLOC_FILL_ZERO);
        sig_expected = sig_vec->fragment_find(0, modulus_bytes_len);
        sig_actual = sig_vec->fragment_find(modulus_bytes_len, modulus_bytes_len);

        // ensure the 2 fragments are contiguous in memory
        assert((sig_expected->pa() + modulus_bytes_len) == sig_actual->pa());

        e = nullptr;
        if (!rsa_key_create_type_is_sign(
                    rsa_testvec.testvec_params.key_create_type())) {
            e = new dp_mem_t(1, modulus_bytes_len,
                             DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.e_mem_type(),
                             0, DP_MEM_ALLOC_NO_FILL);
        }

        /*
         * Allocate salt_val and init to zero length (in case it isn't used by the test)
         */
        salt_val = new dp_mem_t(1, modulus_bytes_len,
                                DP_MEM_ALIGN_NONE, rsa_testvec.testvec_params.msg_mem_type(),
                                0, DP_MEM_ALLOC_NO_FILL);
        salt_val->content_size_set(0);
    }

    ~rsa_msg_repr_t()
    {
        if (rsa_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (rsa_testvec.test_success || !rsa_testvec.hw_started) {
                if (crypto_rsa) delete crypto_rsa;
                if (salt_val) delete salt_val;
                if (e) delete e;
                if (sig_vec) delete sig_vec;
                if (msg_vec) delete msg_vec;
            }
        }
    }

    friend class rsa_testvec_t;

private:
    rsa_testvec_t&              rsa_testvec;
    u_long                      modulus_bytes_len;
    string                      sha_algo;
    dp_mem_t                    *sig_vec;
    dp_mem_t                    *sig_expected;  // fragment of sig_vec
    dp_mem_t                    *sig_actual;    // fragment of sig_vec
    dp_mem_t                    *msg_vec;
    dp_mem_t                    *msg_expected;  // fragment of msg_vec
    dp_mem_t                    *msg_actual;    // fragment of msg_vec
    dp_mem_t                    *e;     // RSA public exponent (only for sig verify)
    dp_mem_t                    *salt_val;      // random #, used by PSS
    rsa_t                       *crypto_rsa;
    parser_token_id_t           failed_parse_token;
    uint32_t                    failure_expected: 1,
                                push_failure    : 1,
                                compl_failure   : 1,
                                verify_failure  : 1;
};

} // namespace crypto_rsa

#endif   // _CRYPTO_RSA_TESTVEC_HPP_

