//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CRYPTO_CERT_STORE_HPP__
#define __CRYPTO_CERT_STORE_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include <openssl/ossl_typ.h>
#include <openssl/ec.h>

namespace hal {

#define HAL_MAX_CRYPTO_CERT_STORE_ELEMS   256
#define CRYPTO_RSA_MAX_KEY_SIZE           256

typedef  uint32_t crypto_cert_id_t;

typedef struct crypto_key_pub_rsa_params_s {
    uint32_t                mod_n_len;
    uint8_t                 mod_n[CRYPTO_RSA_MAX_KEY_SIZE];
    uint32_t                e_len;
    uint8_t                 e[CRYPTO_RSA_MAX_KEY_SIZE];
} crypto_key_pub_rsa_params_t;

typedef struct crypto_key_pub_ec_params_s {
    const EC_GROUP          *group;
    const EC_POINT          *point;
} crypto_key_pub_ec_params_t;

typedef struct crypto_key_pub_info_s {
    uint32_t                key_type; // key type based on openssl
    uint32_t                key_len;
    union {
        crypto_key_pub_rsa_params_t  rsa_params;
        crypto_key_pub_ec_params_t   ec_params;
    } u;
} crypto_key_pub_info_t;

typedef struct crypto_cert_s {
    hal_spinlock_t          slock;       // lock to protect this structure
    crypto_cert_id_t        cert_id;     // id
    X509                    *x509_cert;  // x509 encoded certificate
    crypto_cert_id_t        next_cert_id; // Next certificate in the chain
    crypto_key_pub_info_t   pub_key;      // Public Key info

    ht_ctxt_t            ht_ctxt;     //id based hash table ctxt
} crypto_cert_t;

static inline crypto_cert_t*
crypto_cert_alloc(void)
{
    crypto_cert_t    *cert;

    cert = (crypto_cert_t *)g_hal_state->crypto_cert_store_slab()->alloc();
    if(cert == NULL) {
        return NULL;
    }
    return cert;
}

static inline crypto_cert_t*
crypto_cert_init(crypto_cert_t *cert)
{
    if(!cert) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&cert->slock, PTHREAD_PROCESS_PRIVATE);
    cert->ht_ctxt.reset();
    return cert;
}

static inline crypto_cert_t*
crypto_cert_alloc_init(void)
{
    return crypto_cert_init(crypto_cert_alloc());
}

static inline hal_ret_t
crypto_cert_free(crypto_cert_t *cert)
{
    HAL_SPINLOCK_DESTROY(&cert->slock);
    hal::delay_delete_to_slab(HAL_SLAB_CRYPTO_CERT_STORE, cert);
    return HAL_RET_OK;
}

static inline crypto_cert_t*
find_cert_by_id(crypto_cert_id_t cert_id)
{
    return (crypto_cert_t *)g_hal_state->crypto_cert_store_id_ht()->lookup(&cert_id);
}

extern void *crypto_cert_store_get_key_func(void *entry);
extern uint32_t crypto_cert_store_compute_hash_func(void *key, uint32_t ht_size);
extern bool crypto_cert_store_compare_key_func(void *key1, void *key2);

} // namespace hal

#endif /* __CRYPTO_CERT_STORE_HPP__ */
