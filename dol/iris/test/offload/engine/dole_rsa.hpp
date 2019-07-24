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
        digest_padded_(nullptr),
        caller_ctx_(nullptr),
        wait_for_completion_(false)
    {
    }

    rsa_ex_data_t&
    key_idx(crypto_asym::key_idx_t key_idx)
    {
        key_idx_ = key_idx;
        return *this;
    }
    rsa_ex_data_t&
    digest_padded(dp_mem_t *digest_padded)
    {
        digest_padded_ = digest_padded;
        return *this;
    }
    rsa_ex_data_t&
    caller_ctx(void *caller_ctx)
    {
        caller_ctx_ = caller_ctx;
        return *this;
    }
    rsa_ex_data_t&
    wait_for_completion(bool wait_for_completion)
    {
        wait_for_completion_ = wait_for_completion;
        return *this;
    }

    crypto_asym::key_idx_t key_idx(void) { return key_idx_; }
    dp_mem_t *digest_padded(void) { return digest_padded_; }
    void *caller_ctx(void) { return caller_ctx_; }
    bool wait_for_completion(void) { return wait_for_completion_; }

private:
    crypto_asym::key_idx_t      key_idx_;
    dp_mem_t                    *digest_padded_;
    void                        *caller_ctx_;
    bool                        wait_for_completion_;
};

RSA_METHOD *rsa_methods_get(void);
void rsa_methods_free(void);

EVP_PKEY *rsa_pkey_load(ENGINE* engine,
                        dole_if::dole_key_t *key,
                        UI_METHOD *ui_method,
                        void *callback_data);

} // namespace dole

#endif /* __DOLE_RSA_HPP__ */
