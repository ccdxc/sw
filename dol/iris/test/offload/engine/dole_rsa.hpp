#ifndef __DOLE_RSA_HPP__
#define __DOLE_RSA_HPP__

#include <openssl/ossl_typ.h>
#include <crypto_asym.hpp>
#include <crypto_rsa.hpp>
#include <dole_if.hpp>
#include "dole.hpp"

namespace dole {

/*
 * RSA extension data for saving user context
 */
class rsa_ex_data_t
{
public:

    rsa_ex_data_t() :
        key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
        n_(nullptr),
        d_e_(nullptr),
        user_ctx_(nullptr)
    {
    }

    rsa_ex_data_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    rsa_ex_data_t&
    n(dp_mem_t *n)
    {
        n_ = n;
        return *this;
    }
    rsa_ex_data_t&
    d_e(dp_mem_t *d_e)
    {
        d_e_ = d_e;
        return *this;
    }
    rsa_ex_data_t&
    user_ctx(void *user_ctx)
    {
        user_ctx_ = user_ctx;
        return *this;
    }

    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *n(void) { return n_; }
    dp_mem_t *d_e(void) { return d_e_; }
    void *user_ctx(void) { return user_ctx_; }

private:
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *n_;            // RSA modulus
    dp_mem_t                    *d_e_;          // RSA public or private exponent
    void                        *user_ctx_;
};

RSA_METHOD *rsa_methods_get(void);
void rsa_methods_free(void);

EVP_PKEY *rsa_pkey_load(ENGINE* engine,
                        dole_if::dole_key_t *key,
                        UI_METHOD *ui_method,
                        void *callback_data);

} // namespace dole

#endif /* __DOLE_RSA_HPP__ */
