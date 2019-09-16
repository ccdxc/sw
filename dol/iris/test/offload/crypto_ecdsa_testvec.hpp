#ifndef _CRYPTO_ECDSA_TESTVEC_HPP_
#define _CRYPTO_ECDSA_TESTVEC_HPP_

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
#include "crypto_ecdsa.hpp"

using namespace tests;

namespace crypto_ecdsa {

class ecdsa_curve_repr_t;
class ecdsa_msg_repr_t;
class ecdsa_testvec_t;

/*
 * Emulate named parameters support for ecdsa_testvec_t constructor
 */
class ecdsa_testvec_params_t
{
public:

    ecdsa_testvec_params_t() :
        key_create_type_(ECDSA_KEY_CREATE_VOID),
        q_mem_type_(DP_MEM_TYPE_HBM),
        d_mem_type_(DP_MEM_TYPE_HBM),
        k_mem_type_(DP_MEM_TYPE_HBM),
        msg_mem_type_(DP_MEM_TYPE_HBM),
        sig_mem_type_(DP_MEM_TYPE_HBM),
        dma_desc_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM)
    {
    }

    ecdsa_testvec_params_t&
    key_create_type(ecdsa_key_create_type_t key_create_type)
    {
        key_create_type_ = key_create_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    q_mem_type(dp_mem_type_t q_mem_type)
    {
        q_mem_type_ = q_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    d_mem_type(dp_mem_type_t d_mem_type)
    {
        d_mem_type_ = d_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    k_mem_type(dp_mem_type_t k_mem_type)
    {
        k_mem_type_ = k_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    msg_mem_type(dp_mem_type_t msg_mem_type)
    {
        msg_mem_type_ = msg_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    sig_mem_type(dp_mem_type_t sig_mem_type)
    {
        sig_mem_type_ = sig_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    dma_desc_mem_type(dp_mem_type_t dma_desc_mem_type)
    {
        dma_desc_mem_type_ = dma_desc_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    ecdsa_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    ecdsa_key_create_type_t key_create_type(void) { return key_create_type_; }
    dp_mem_type_t q_mem_type(void) { return q_mem_type_; }
    dp_mem_type_t d_mem_type(void) { return d_mem_type_; }
    dp_mem_type_t k_mem_type(void) { return k_mem_type_; }
    dp_mem_type_t msg_mem_type(void) { return msg_mem_type_; }
    dp_mem_type_t sig_mem_type(void) { return sig_mem_type_; }
    dp_mem_type_t dma_desc_mem_type(void) { return dma_desc_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    ecdsa_key_create_type_t     key_create_type_;
    dp_mem_type_t               q_mem_type_;
    dp_mem_type_t               d_mem_type_;
    dp_mem_type_t               k_mem_type_;
    dp_mem_type_t               msg_mem_type_;
    dp_mem_type_t               sig_mem_type_;
    dp_mem_type_t               dma_desc_mem_type_;
    dp_mem_type_t               status_mem_type_;
    offload_base_params_t       base_params_;
};

/*
 * Emulate named parameters support for ecdsa_testvec_t pre_push
 */
class ecdsa_testvec_pre_push_params_t
{
public:

    ecdsa_testvec_pre_push_params_t() {}

    ecdsa_testvec_pre_push_params_t&
    scripts_dir(const string& scripts_dir)
    {
        scripts_dir_.assign(scripts_dir);
        return *this;
    }
    ecdsa_testvec_pre_push_params_t&
    testvec_fname(const string& testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }
    ecdsa_testvec_pre_push_params_t&
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
 * Emulate named parameters support for ecdsa_testvec_t push
 */
class ecdsa_testvec_push_params_t
{
public:

    ecdsa_testvec_push_params_t() :
        ecdsa_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    ecdsa_testvec_push_params_t&
    ecdsa_ring(acc_ring_t *ecdsa_ring)
    {
        ecdsa_ring_ = ecdsa_ring;
        return *this;
    }
    ecdsa_testvec_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    ecdsa_testvec_push_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }

    acc_ring_t *ecdsa_ring(void) { return ecdsa_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }

private:
    acc_ring_t                  *ecdsa_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
};

/*
 * Crypto ECDSA test vector
 */
class ecdsa_testvec_t {

public:
    ecdsa_testvec_t(ecdsa_testvec_params_t& params);
    ~ecdsa_testvec_t();

    friend class ecdsa_curve_repr_t;
    friend class ecdsa_msg_repr_t;

    bool pre_push(ecdsa_testvec_pre_push_params_t& pre_params);
    bool push(ecdsa_testvec_push_params_t& push_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    void rsp_file_output(void);

private:
    ecdsa_testvec_params_t        testvec_params;
    ecdsa_testvec_pre_push_params_t pre_params;
    ecdsa_testvec_push_params_t   push_params;
    testvec_parser_t            *testvec_parser;
    testvec_output_t            *rsp_output;
    vector<shared_ptr<ecdsa_curve_repr_t>> curve_repr_vec;

    uint32_t                    num_test_failures;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Crypto ECDSA curve representative
 */
class ecdsa_curve_repr_t {

public:
    ecdsa_curve_repr_t(ecdsa_testvec_t& ecdsa_testvec,
                       u_long P_bytes_len,
                       u_long P_expanded_len) :
        ecdsa_testvec(ecdsa_testvec),
        P_bytes_len(P_bytes_len),
        P_expanded_len(P_expanded_len),
        failed_parse_token(PARSE_TOKEN_ID_VOID)
    {
        /*
         * Normally we'd allocate a dp_mem vector here to achieve memory 
         * contiguity but if the underlying mem_type is HBM, the corresponding
         * allocation library may pad out the length and defeat contiguity.
         * Hence, we have to resort to allocating size multiples so that
         * contiguity can still be obtained with fragment_find().
         */
        domain_vec = new dp_mem_t(1, P_expanded_len *
                                     crypto_asym::ECDSA_NUM_KEY_DOMAIN_FIELDS,
                         DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.d_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        p  = domain_vec->fragment_find(P_expanded_len * 0, P_expanded_len);
        n  = domain_vec->fragment_find(P_expanded_len * 1, P_expanded_len);
        xg = domain_vec->fragment_find(P_expanded_len * 2, P_expanded_len);
        yg = domain_vec->fragment_find(P_expanded_len * 3, P_expanded_len);
        a  = domain_vec->fragment_find(P_expanded_len * 4, P_expanded_len);
        b  = domain_vec->fragment_find(P_expanded_len * 5, P_expanded_len);
    }

    ~ecdsa_curve_repr_t()
    {
        if (ecdsa_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (ecdsa_testvec.test_success || !ecdsa_testvec.hw_started) {
                if (domain_vec) delete domain_vec;
            }
        }
    }

    friend class ecdsa_testvec_t;

private:
    ecdsa_testvec_t&            ecdsa_testvec;
    u_long                      P_bytes_len;    // size of the finite field
    u_long                      P_expanded_len; // required HW expanded size
    string                      curve_name;
    string                      sha;
    string                      hash_algo;
    parser_token_id_t           failed_parse_token;

    // Generated domain parameters from curve_name;
    // These are fragments of domain_vec to ensure contiguity.
    // Note that HW also expects the key params to consist of
    // these 6 items (p,n,xg,yg,a,b) in one contiguous chunk.
    dp_mem_t                    *domain_vec;
    dp_mem_t                    *p;
    dp_mem_t                    *n;
    dp_mem_t                    *xg;
    dp_mem_t                    *yg;
    dp_mem_t                    *a;
    dp_mem_t                    *b;

    vector<shared_ptr<ecdsa_msg_repr_t>> msg_repr_vec;
};

/*
 * Crypto ECDSA message representative
 */
class ecdsa_msg_repr_t {

public:
    ecdsa_msg_repr_t(ecdsa_testvec_t& ecdsa_testvec,
                     u_long P_bytes_len,
                     u_long P_expanded_len) :
        ecdsa_testvec(ecdsa_testvec),
        crypto_ecdsa(nullptr),
        failed_parse_token(PARSE_TOKEN_ID_VOID),
        failure_expected(false),
        push_failure(false),
        compl_failure(false),
        verify_failure(false)
    {
        assert(P_expanded_len);

        pub_key_vec = new dp_mem_t(1, P_expanded_len *
                          crypto_asym::ECDSA_NUM_PUB_KEY_FIELDS,
                          DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.q_mem_type(),
                          0, DP_MEM_ALLOC_NO_FILL);
        qx = pub_key_vec->fragment_find(P_expanded_len * 0, P_expanded_len);
        qy = pub_key_vec->fragment_find(P_expanded_len * 1, P_expanded_len);

        k = new dp_mem_t(1, P_expanded_len,
                         DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.k_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        sig_expected_vec = new dp_mem_t(1, P_expanded_len *
                                           crypto_asym::ECDSA_NUM_SIG_FIELDS,
                           DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.sig_mem_type(),
                           0, DP_MEM_ALLOC_NO_FILL);
        r_expected = sig_expected_vec->fragment_find(P_expanded_len * 0, P_expanded_len);
        s_expected = sig_expected_vec->fragment_find(P_expanded_len * 1, P_expanded_len);

        d = new dp_mem_t(1, P_expanded_len,
                         DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.d_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        msg = new dp_mem_t(1, CRYPTO_ECDSA_MSG_ACTUAL_SIZE_MAX,
                         DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.msg_mem_type(),
                         0, DP_MEM_ALLOC_NO_FILL);
        sig_actual_vec = new dp_mem_t(1, P_expanded_len *
                                         crypto_asym::ECDSA_NUM_SIG_FIELDS,
                         DP_MEM_ALIGN_NONE, ecdsa_testvec.testvec_params.sig_mem_type(),
                         0, DP_MEM_ALLOC_FILL_ZERO);
        r_actual = sig_actual_vec->fragment_find(P_expanded_len * 0, P_expanded_len);
        s_actual = sig_actual_vec->fragment_find(P_expanded_len * 1, P_expanded_len);
    }

    ~ecdsa_msg_repr_t()
    {
        if (ecdsa_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (ecdsa_testvec.test_success || !ecdsa_testvec.hw_started) {
                if (crypto_ecdsa) delete crypto_ecdsa;
                if (sig_actual_vec) delete sig_actual_vec;
                if (sig_expected_vec) delete sig_expected_vec;
                if (pub_key_vec) delete pub_key_vec;
                if (msg) delete msg;
                if (d) delete d;
                if (k) delete k;
            }
        }
    }

    friend class ecdsa_testvec_t;

private:
    ecdsa_testvec_t&            ecdsa_testvec;
    dp_mem_t                    *pub_key_vec;
    dp_mem_t                    *qx;            // (qx, qy) are fragments of pub_key_vec
    dp_mem_t                    *qy;
    dp_mem_t                    *k;             // fresh random value
    dp_mem_t                    *sig_expected_vec;
    dp_mem_t                    *r_expected;    // fragment of sig_expected_vec
    dp_mem_t                    *s_expected;    // fragment of sig_expected_vec
    dp_mem_t                    *d;             // ECDSA private exponent
    dp_mem_t                    *msg;

    dp_mem_t                    *sig_actual_vec;
    dp_mem_t                    *r_actual;      // fragment of sig_actual_vec
    dp_mem_t                    *s_actual;      // fragment of sig_actual_vec

    ecdsa_t                     *crypto_ecdsa;
    parser_token_id_t           failed_parse_token;
    uint32_t                    failure_expected: 1,
                                push_failure    : 1,
                                compl_failure   : 1,
                                verify_failure  : 1;
};

} // namespace crypto_ecdsa

#endif  // _CRYPTO_ECDSA_TESTVEC_HPP_

