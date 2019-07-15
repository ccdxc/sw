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
extern BIO     *dole_bio;

typedef EVP_MD          dole_evp_md_t;

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
        d_e_(nullptr)
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

    int key_type(void) { return key_type_; }
    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d_e(void) { return d_e_; }

private:
    int                         key_type_;
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_e_;          // RSA public or private exponent
};

bool init(const char *engine_path);

const dole_evp_md_t *hash_algo_find(const string& hash_algo);

const dole_evp_md_t *rsa_msg_digest_push(const string& hash_algo,
                                         dp_mem_t *msg,
                                         dp_mem_t *digest_output);
bool rsa_sig_push(void *user_ctx,
                  const dole_evp_md_t *md,
                  crypto_asym::key_idx_t key_idx,
                  dp_mem_t *n,
                  dp_mem_t *d_e,
                  dp_mem_t *msg_digest,
                  dp_mem_t *sig_actual);

} // namespace dole_intf

#endif /* __DOLE_IF_HPP__ */

