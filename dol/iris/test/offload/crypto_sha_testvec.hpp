#ifndef _CRYPTO_SHA_TESTVEC_HPP_
#define _CRYPTO_SHA_TESTVEC_HPP_

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
#include "crypto_sha.hpp"

using namespace tests;

namespace crypto_sha {

/*
 * Default CAVP Montecarlo iterations
 */
#define SHA_MONTECARLO_ITERS_MAX        100000
#define SHA_MONTECARLO_RESULT_EPOCH     1000
#define SHA_MONTECARLO_RESULT_LIVENESS  10

/*
 * Default CAVP message size in multiples of seed size
 */
#define SHA_MONTECARLO_SEED_MULT_SHA3   1
#define SHA_MONTECARLO_SEED_MULT_SHA1   3
#define SHA_MONTECARLO_SEED_MULT_MAX    SHA_MONTECARLO_SEED_MULT_SHA1


class sha_test_repr_t;
class sha_msg_repr_t;
class sha_testvec_t;

/*
 * Emulate named parameters support for sha_testvec_t constructor
 */
class sha_testvec_params_t
{
public:

    sha_testvec_params_t() :
        crypto_symm_type_(crypto_symm::CRYPTO_SYMM_TYPE_SHA),
        msg_desc_mem_type_(DP_MEM_TYPE_HBM),
        msg_digest_mem_type_(DP_MEM_TYPE_HBM),
        status_mem_type_(DP_MEM_TYPE_HBM),
        doorbell_mem_type_(DP_MEM_TYPE_HBM),
        montecarlo_iters_max_(SHA_MONTECARLO_ITERS_MAX),
        montecarlo_result_epoch_(SHA_MONTECARLO_RESULT_EPOCH)
    {
    }

    sha_testvec_params_t&
    crypto_symm_type(crypto_symm::crypto_symm_type_t crypto_symm_type)
    {
        crypto_symm_type_ = crypto_symm_type;
        return *this;
    }
    sha_testvec_params_t&
    msg_desc_mem_type(dp_mem_type_t msg_desc_mem_type)
    {
        msg_desc_mem_type_ = msg_desc_mem_type;
        return *this;
    }
    sha_testvec_params_t&
    msg_digest_mem_type(dp_mem_type_t msg_digest_mem_type)
    {
        msg_digest_mem_type_ = msg_digest_mem_type;
        return *this;
    }
    sha_testvec_params_t&
    status_mem_type(dp_mem_type_t status_mem_type)
    {
        status_mem_type_ = status_mem_type;
        return *this;
    }
    sha_testvec_params_t&
    doorbell_mem_type(dp_mem_type_t doorbell_mem_type)
    {
        doorbell_mem_type_ = doorbell_mem_type;
        return *this;
    }
    sha_testvec_params_t&
    montecarlo_iters_max(uint32_t montecarlo_iters_max)
    {
        montecarlo_iters_max_ = montecarlo_iters_max;
        return *this;
    }
    sha_testvec_params_t&
    montecarlo_result_epoch(uint32_t montecarlo_result_epoch)
    {
        montecarlo_result_epoch_ = montecarlo_result_epoch;
        return *this;
    }
    sha_testvec_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    crypto_symm::crypto_symm_type_t crypto_symm_type(void) { return crypto_symm_type_; }
    dp_mem_type_t msg_desc_mem_type(void) { return msg_desc_mem_type_; }
    dp_mem_type_t msg_digest_mem_type(void) { return msg_digest_mem_type_; }
    dp_mem_type_t status_mem_type(void) { return status_mem_type_; }
    dp_mem_type_t doorbell_mem_type(void) { return doorbell_mem_type_; }
    uint32_t montecarlo_iters_max(void) { return montecarlo_iters_max_; }
    uint32_t montecarlo_result_epoch(void) { return montecarlo_result_epoch_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    crypto_symm::crypto_symm_type_t crypto_symm_type_;
    dp_mem_type_t               msg_desc_mem_type_;
    dp_mem_type_t               msg_digest_mem_type_;
    dp_mem_type_t               status_mem_type_;
    dp_mem_type_t               doorbell_mem_type_;
    uint32_t                    montecarlo_iters_max_;
    uint32_t                    montecarlo_result_epoch_;
    offload_base_params_t       base_params_;

};

/*
 * Emulate named parameters support for sha_testvec_t pre_push
 */
class sha_testvec_pre_push_params_t
{
public:

    sha_testvec_pre_push_params_t() {}

    sha_testvec_pre_push_params_t&
    scripts_dir(const string& scripts_dir)
    {
        scripts_dir_.assign(scripts_dir);
        return *this;
    }
    sha_testvec_pre_push_params_t&
    testvec_fname(const string& testvec_fname)
    {
        testvec_fname_.assign(testvec_fname);
        return *this;
    }
    sha_testvec_pre_push_params_t&
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
 * Emulate named parameters support for sha_testvec_t push
 */
class sha_testvec_push_params_t
{
public:

    sha_testvec_push_params_t() :
        sha_ring_(nullptr),
        push_type_(ACC_RING_PUSH_HW_DIRECT),
        seq_qid_(0)
    {
    }

    sha_testvec_push_params_t&
    sha_ring(acc_ring_t *sha_ring)
    {
        sha_ring_ = sha_ring;
        return *this;
    }
    sha_testvec_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    sha_testvec_push_params_t&
    seq_qid(uint32_t seq_qid)
    {
        seq_qid_ = seq_qid;
        return *this;
    }

    acc_ring_t *sha_ring(void) { return sha_ring_; }
    acc_ring_push_t push_type(void) { return push_type_; }
    uint32_t seq_qid(void) { return seq_qid_; }

private:
    acc_ring_t                  *sha_ring_;
    acc_ring_push_t             push_type_;
    uint32_t                    seq_qid_;
};

/*
 * Crypto SHA Montecarlo result
 */
class sha_montecarlo_result_t {

public:
    sha_montecarlo_result_t() :
        repr_nbytes(0)
    {
    }
    ~sha_montecarlo_result_t()
    {
        for (size_t idx = 0; idx < count(); idx++) {
            delete [] result_vec.at(idx);
        }
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
             dst->write_thru();
             return true;
         }
         return false;
    }

    uint32_t size(void)
    {
         return repr_nbytes;
    }

private:
    vector<uint8_t *>           result_vec;
    uint32_t                    repr_nbytes;
};

/*
 * Crypto SHA test vector
 */
class sha_testvec_t {

public:
    sha_testvec_t(sha_testvec_params_t& params);
    ~sha_testvec_t();

    friend class sha_test_repr_t;
    friend class sha_msg_repr_t;

    bool pre_push(sha_testvec_pre_push_params_t& pre_params);
    bool push(sha_testvec_push_params_t& push_params);
    bool post_push(void);
    bool completion_check(void);
    bool full_verify(void);
    void rsp_file_output(void);

private:
    bool is_montecarlo(sha_msg_repr_t *msg_repr);
    bool shall_execute(sha_msg_repr_t *msg_repr);
    bool montecarlo_msg_set(sha_msg_repr_t *msg_repr,
                            dp_mem_t *seed0,
                            dp_mem_t *seed1,
                            dp_mem_t *seed2);
    void montecarlo_execute(sha_msg_repr_t *msg_repr);

    sha_testvec_params_t        testvec_params;
    sha_testvec_pre_push_params_t pre_params;
    sha_testvec_push_params_t   push_params;
    testvec_parser_t            *testvec_parser;
    testvec_output_t            *rsp_output;
    vector<shared_ptr<sha_test_repr_t>> test_repr_vec;

    uint32_t                    num_test_failures;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Crypto SHA test representative
 */
class sha_test_repr_t {

public:
    sha_test_repr_t(sha_testvec_t& sha_testvec) :
        sha_testvec(sha_testvec),
        sha_nbytes(0),
        failed_parse_token(PARSE_TOKEN_ID_VOID)
    {
    }

    ~sha_test_repr_t()
    {
        if (sha_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (sha_testvec.test_success || !sha_testvec.hw_started) {
            }
        }
    }

    friend class sha_testvec_t;
    friend class sha_msg_repr_t;

private:
    sha_testvec_t&              sha_testvec;
    string                      sha_algo;
    u_long                      sha_nbytes;
    parser_token_id_t           failed_parse_token;
    vector<shared_ptr<sha_msg_repr_t>> msg_repr_vec;
};

/*
 * Crypto SHA message representative
 */
class sha_msg_repr_t {

public:
    sha_msg_repr_t(sha_testvec_t& sha_testvec,
                   u_long sha_nbytes) :
        sha_testvec(sha_testvec),
        sha_nbytes(sha_nbytes),
        msg_nbits(0),
        msg(nullptr),
        crypto_sha(nullptr),
        failed_parse_token(PARSE_TOKEN_ID_VOID),
        failure_expected(false),
        push_failure(false),
        compl_failure(false),
        verify_failure(false)
    {
        assert(sha_nbytes);
        seed = new dp_mem_t(1, sha_nbytes, DP_MEM_ALIGN_SPEC,
                            sha_testvec.testvec_params.msg_digest_mem_type(),
                            CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                            DP_MEM_ALLOC_NO_FILL);
        seed->content_size_set(0);
        md_expected = new dp_mem_t(1, sha_nbytes, DP_MEM_ALIGN_SPEC,
                                   sha_testvec.testvec_params.msg_digest_mem_type(),
                                   CRYPTO_SYMM_HASH_OUTPUT_ALIGNMENT,
                                   DP_MEM_ALLOC_NO_FILL);
        md_expected->content_size_set(0);
        md_actual = new dp_mem_t(1, sha_nbytes, DP_MEM_ALIGN_SPEC,
                                 sha_testvec.testvec_params.msg_digest_mem_type(),
                                 CRYPTO_SYMM_HASH_OUTPUT_ALIGNMENT,
                                 DP_MEM_ALLOC_FILL_ZERO);
        md_actual->content_size_set(0);

        /*
         * Contiguous messages, each of seed size, for Montecarlo test
         */
        msg_mct_vec = new dp_mem_t(1, sha_nbytes * SHA_MONTECARLO_SEED_MULT_MAX,
                                   DP_MEM_ALIGN_SPEC,
                                   sha_testvec.testvec_params.msg_digest_mem_type(),
                                   CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                                   DP_MEM_ALLOC_NO_FILL);
        msg_mct0 = msg_mct_vec->fragment_find(sha_nbytes * 0, sha_nbytes);
        msg_mct1 = msg_mct_vec->fragment_find(sha_nbytes * 1, sha_nbytes);
        msg_mct2 = msg_mct_vec->fragment_find(sha_nbytes * 2, sha_nbytes);
        msg_mct_vec->content_size_set(0);
    }

    bool msg_alloc(void)
    {
        uint32_t alloc_nbytes;

        /*
         * Ensure we allocate at least 1 byte
         */
        alloc_nbytes = msg_nbits ?
                       (msg_nbits + BITS_PER_BYTE - 1) / BITS_PER_BYTE :
                       1;
        msg = new dp_mem_t(1, alloc_nbytes, DP_MEM_ALIGN_SPEC,
                           sha_testvec.testvec_params.msg_digest_mem_type(),
                           CRYPTO_SYMM_MSG_INPUT_ALIGNMENT,
                           DP_MEM_ALLOC_NO_FILL);
        msg->content_size_set(0);
        return true;
    }

    ~sha_msg_repr_t()
    {
        if (sha_testvec.testvec_params.base_params().destructor_free_buffers()) {
            if (sha_testvec.test_success || !sha_testvec.hw_started) {
                if (crypto_sha) delete crypto_sha;
                if (seed) delete seed;
                if (msg_mct_vec) delete msg_mct_vec;
                if (msg) delete msg;
                if (md_actual) delete md_actual;
                if (md_expected) delete md_expected;
            }
        }
    }

    friend class sha_testvec_t;

private:
    sha_testvec_t&              sha_testvec;
    u_long                      sha_nbytes;
    u_long                      msg_nbits;
    dp_mem_t                    *seed;
    dp_mem_t                    *msg;
    dp_mem_t                    *msg_mct_vec;
    dp_mem_t                    *msg_mct0;      // fragments of msg_mct_vec
    dp_mem_t                    *msg_mct1;      // " "
    dp_mem_t                    *msg_mct2;      // " "
    dp_mem_t                    *md_expected;
    dp_mem_t                    *md_actual;
    sha_t                       *crypto_sha;

    sha_montecarlo_result_t     montecarlo_expected;
    sha_montecarlo_result_t     montecarlo_actual;
    parser_token_id_t           failed_parse_token;
    uint32_t                    failure_expected: 1,
                                push_failure    : 1,
                                compl_failure   : 1,
                                verify_failure  : 1;
};

} // namespace crypto_sha

#endif   // _CRYPTO_SHA_TESTVEC_HPP_

