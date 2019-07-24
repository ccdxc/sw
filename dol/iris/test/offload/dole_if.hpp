#ifndef __DOLE_IF_HPP__
#define __DOLE_IF_HPP__

#include <map>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include "logger.hpp"
#include "nic/include/base.hpp"
#include "crypto_asym.hpp"

using namespace std;

namespace dole_if {

extern ENGINE  *dole;
extern BIO     *dole_if_bio;

typedef EVP_MD dole_evp_md_t;

/*
 * DOL engine generic key
 */
class dole_key_t
{
public:
    dole_key_t() :
        key_type_(EVP_PKEY_NONE),
        key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
        n_(nullptr),
        d_e_(nullptr),
        digest_padded_(nullptr),
        wait_for_completion_(false)
    {
    }

    dole_key_t&
    key_type(int key_type)
    {
        key_type_ = key_type;
        return *this;
    }
    dole_key_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    dole_key_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    dole_key_t&
    d_e(dp_mem_t *d_e)
    {
        d_e_ = d_e;
        return *this;
    }
    dole_key_t&
    digest_padded(dp_mem_t *digest_padded)
    {
        digest_padded_ = digest_padded;
        return *this;
    }
    dole_key_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }

    int key_type(void) { return key_type_; }
    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d_e(void) { return d_e_; }
    dp_mem_t *digest_padded(void) { return digest_padded_; }
    bool wait_for_completion(void) { return wait_for_completion_; }

private:
    int                         key_type_;
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_e_;          // RSA public or private exponent
    dp_mem_t                    *digest_padded_;
    bool                        wait_for_completion_;
};

/*
 * DOL engine RSA digest parameters
 */
class rsa_digest_params_t
{
public:
    rsa_digest_params_t() :
        msg_(nullptr),
        digest_(nullptr)
    {
    }

    rsa_digest_params_t&
    hash_algo(const string& hash_algo)
    {
        hash_algo_ = hash_algo;
        return *this;
    }
    rsa_digest_params_t&
    msg(dp_mem_t *msg)
    {
        msg_ = msg;
        return *this;
    }
    rsa_digest_params_t&
    digest(dp_mem_t *digest)
    {
        digest_ = digest;
        return *this;
    }

    const string& hash_algo(void) { return hash_algo_; }
    dp_mem_t *msg(void) { return msg_; }
    dp_mem_t *digest(void) { return digest_; }

private:
    string                      hash_algo_;
    dp_mem_t                    *msg_;
    dp_mem_t                    *digest_;
};

/*
 * DOL engine RSA sign parameters
 */
class rsa_sign_params_t
{
public:
    rsa_sign_params_t() :
        md_(nullptr),
        pad_mode_(RSA_NO_PADDING),
        key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
        n_(nullptr),
        d_e_(nullptr),
        digest_(nullptr),
        digest_padded_(nullptr),
        sig_actual_(nullptr),
        rsa_(nullptr),
        failure_expected_(false),
        wait_for_completion_(false)
    {
    }

    rsa_sign_params_t&
    md(const dole_evp_md_t *md)
    {
        md_ = md;
        return *this;
    }
    rsa_sign_params_t&
    pad_mode(int pad_mode)
    {
        pad_mode_ = pad_mode;
        return *this;
    }
    rsa_sign_params_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    rsa_sign_params_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    rsa_sign_params_t&
    d_e(dp_mem_t *d_e)
    {
        d_e_ = d_e;
        return *this;
    }
    rsa_sign_params_t&
    digest(dp_mem_t *digest)
    {
        digest_ = digest;
        return *this;
    }
    rsa_sign_params_t&
    digest_padded(dp_mem_t *digest_padded)
    {
        digest_padded_ = digest_padded;
        return *this;
    }
    rsa_sign_params_t&
    sig_actual(dp_mem_t *sig_actual)
    {
        sig_actual_ = sig_actual;
        return *this;
    }
    rsa_sign_params_t&
    rsa(void *rsa)
    {
        rsa_ = rsa;
        return *this;
    }
    rsa_sign_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }
    rsa_sign_params_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }

    const dole_evp_md_t *md(void) { return md_; }
    int pad_mode(void) { return pad_mode_; }
    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d_e(void) { return d_e_; }
    dp_mem_t *digest(void) { return digest_; }
    dp_mem_t *digest_padded(void) { return digest_padded_; }
    dp_mem_t *sig_actual(void) { return sig_actual_; }
    void *rsa(void) { return rsa_; }
    bool failure_expected(void) { return failure_expected_; }
    bool wait_for_completion(void) { return wait_for_completion_; }

private:
    const dole_evp_md_t         *md_;
    int                         pad_mode_;
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_e_;          // RSA public or private exponent
    dp_mem_t                    *digest_;
    dp_mem_t                    *digest_padded_;
    dp_mem_t                    *sig_actual_;
    void                        *rsa_;
    bool                        failure_expected_;
    bool                        wait_for_completion_;
};

/*
 * DOL engine RSA verify parameters
 */
class rsa_verify_params_t
{
public:
    rsa_verify_params_t() :
        md_(nullptr),
        pad_mode_(RSA_NO_PADDING),
        key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
        n_(nullptr),
        d_e_(nullptr),
        sig_expected_(nullptr),
        digest_(nullptr),
        digest_padded_(nullptr),
        rsa_(nullptr),
        failure_expected_(false),
        wait_for_completion_(false)
    {
    }

    rsa_verify_params_t&
    md(const dole_evp_md_t *md)
    {
        md_ = md;
        return *this;
    }
    rsa_verify_params_t&
    pad_mode(int pad_mode)
    {
        pad_mode_ = pad_mode;
        return *this;
    }
    rsa_verify_params_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    rsa_verify_params_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    rsa_verify_params_t&
    d_e(dp_mem_t *d_e)
    {
        d_e_ = d_e;
        return *this;
    }
    rsa_verify_params_t&
    sig_expected(dp_mem_t *sig_expected)
    {
        sig_expected_ = sig_expected;
        return *this;
    }
    rsa_verify_params_t&
    digest(dp_mem_t *digest)
    {
        digest_ = digest;
        return *this;
    }
    rsa_verify_params_t&
    digest_padded(dp_mem_t *digest_padded)
    {
        digest_padded_ = digest_padded;
        return *this;
    }
    rsa_verify_params_t&
    rsa(void *rsa)
    {
        rsa_ = rsa;
        return *this;
    }
    rsa_verify_params_t&
    failure_expected(bool failure_expected)
    {
        failure_expected_ = failure_expected;
        return *this;
    }
    rsa_verify_params_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }

    const dole_evp_md_t *md(void) { return md_; }
    int pad_mode(void) { return pad_mode_; }
    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d_e(void) { return d_e_; }
    dp_mem_t *sig_expected(void) { return sig_expected_; }
    dp_mem_t *digest(void) { return digest_; }
    dp_mem_t *digest_padded(void) { return digest_padded_; }
    void *rsa(void) { return rsa_; }
    bool failure_expected(void) { return failure_expected_; }
    bool wait_for_completion(void) { return wait_for_completion_; }

private:
    const dole_evp_md_t         *md_;
    int                         pad_mode_;
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_e_;          // RSA public or private exponent
    dp_mem_t                    *sig_expected_;
    dp_mem_t                    *digest_;
    dp_mem_t                    *digest_padded_;
    void                        *rsa_;
    bool                        failure_expected_;
    bool                        wait_for_completion_;
};

bool init(const char *engine_path);

const dole_evp_md_t *hash_algo_find(const string& hash_algo);
bool msg_optional_pad(dp_mem_t *msg,
                      uint32_t tolen);

const dole_evp_md_t *rsa_digest(rsa_digest_params_t& params);
bool rsa_sign(rsa_sign_params_t& params);
bool rsa_verify(rsa_verify_params_t& params);

} // namespace dole_intf

#endif /* __DOLE_IF_HPP__ */

